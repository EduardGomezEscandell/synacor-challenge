#include "virtual_machine.h"
#include "address.h"
#include "instruction.h"
#include "word.h"
#include "serializer.h"

#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <limits>

void VirtualMachine::LoadMemory(program_file_t& source)
{
    m_memory.load(source, m_stack_ptr);
}

/**
 * @brief Removes items from a map according to a predicate
 */
template< typename TMap, typename TPredicate>
constexpr void map_erase_if(TMap& items, const TPredicate& predicate)
{
    for(auto it = items.begin(); it != items.end();)
    {
        if(predicate(*it->second)) it = items.erase(it);
        else ++it;
    }
}


void VirtualMachine::Run()
{
    StackInit();

    while(!m_flags.Is(Flags::HALTED | Flags::ERROR))
    {
        ExecuteNextInstruction();

        std::erase_if(m_attached_processes, [](auto& process_ptr){ return (*process_ptr)(); });
    }
}

void VirtualMachine::Print() const
{
    std::cout << "Registers:\n";
    std::cout << "-instr : " 
              << m_instr_ptr.get().hex_dump() <<" ("
              << InstructionData::InstructionName(InstructionData::to_opcode(m_memory[m_instr_ptr]))
              << ")\n";

    for(size_t i=0; i<8; ++i)
    {
        std::cout << "-data " << (char) ('a' + i) << ": " 
                  << m_registers[i].hex_dump() 
                  << " | " << std::setw(5) << std::setfill(' ') << m_registers[i].to_int()
                  << " | " << m_registers[i].hi() << m_registers[i].lo() << '\n';
    }

    std::cout << "\nFlags:\n";
    std::cout << "- HALTED   : " << m_flags.Is(Flags::HALTED) << '\n';
    std::cout << "- ERROR    : " << m_flags.Is(Flags::ERROR) << '\n';
    std::cout << "- BAD_INT  : " << m_flags.Is(Flags::BAD_INTEGER) << '\n';
    std::cout << "- STACK_UF : " << m_flags.Is(Flags::STACK_UNDERFLOW) << '\n';
    std::cout << "- W_ON_LIT : " << m_flags.Is(Flags::WRITE_ON_LITERAL) << '\n';
    std::cout << "- INTERRUPT: " << m_flags.Is(Flags::INTERRUPT) << '\n';

    std::cout << "\nMemory around instruction pointer:\n";
    const std::size_t instr_ptr_row = m_instr_ptr.get().to_int() / 8;
    memory().hex_dump(instr_ptr_row, instr_ptr_row+2, m_instr_ptr.get().to_int());

    std::cout << "\nStack:\n";
    const std::size_t stack_base_ptr_row = m_stack_base_ptr.get().to_int() / 8;
    const std::size_t stack_ptr_row = m_stack_ptr.get().to_int() / 8;
    memory().hex_dump(stack_base_ptr_row, stack_ptr_row+1, m_stack_ptr.get().to_int());
    std::cout << "\n\n";

    std::cout << "Attached processes:\n";
    for(auto const& process_ptr : m_attached_processes) {
        std::cout << "- " << process_ptr->Name() << '\n';;
    }

    std::cout << std::endl;
}

constexpr Word& VirtualMachine::DecodeRegisterUnsafe(Word const& w) noexcept
{
    return m_registers[w.lo()];
}

constexpr Word const& VirtualMachine::DecodeRegisterUnsafe(Word const& w) const noexcept
{
    return m_registers[w.lo()];
}

constexpr Word& VirtualMachine::DecodeRegister(Word& w)
{
    const auto wordtype = InstructionData::to_wordtype(w);
    switch(wordtype)
    {
        case InstructionData::REGISTER:
            return DecodeRegisterUnsafe(w);

        case InstructionData::LITERAL:
            m_flags.Set(Flags::WRITE_ON_LITERAL | Flags::ERROR);
            return m_nul_register;
        
        case InstructionData::INVALID:
            m_flags.Set(Flags::BAD_INTEGER | Flags::ERROR);
            return m_nul_register;
    }

    m_flags.Set(Flags::ERROR);
    return m_nul_register;
}

constexpr Word const& VirtualMachine::GetValue(Word const& w)
{
    auto wordtype = InstructionData::to_wordtype(w);
    switch(wordtype)
    {
        case InstructionData::REGISTER: return DecodeRegisterUnsafe(w);
        case InstructionData::LITERAL:  return w;
        case InstructionData::INVALID:
            m_flags.Set(Flags::BAD_INTEGER | Flags::ERROR);
            return m_nul_register;
    }

    m_flags.Set(Flags::ERROR);
    return m_nul_register;
}

constexpr void VirtualMachine::StackInit() noexcept
{
    m_stack_base_ptr = Address((m_stack_ptr.get().to_int() / 8 + 1) * 8); // Starts at next line
    m_stack_ptr = m_stack_base_ptr;
}

constexpr void VirtualMachine::StackPush(Word const& val) noexcept
{
    m_memory[m_stack_ptr++] = val;
}

constexpr Word VirtualMachine::StackPop() noexcept
{
    if(m_stack_ptr == m_stack_base_ptr)
    {
        m_flags.Set(Flags::STACK_UNDERFLOW | Flags::ERROR);
    }

    return m_memory[--m_stack_ptr];
}

template<typename TOperator>
constexpr void VirtualMachine::ExecuteBinaryOp(TOperator const& Op) noexcept
{
    Word& a = DecodeRegister(m_memory[++m_instr_ptr]);
    const Word b = GetValue(m_memory[++m_instr_ptr]);
    const Word c = GetValue(m_memory[++m_instr_ptr]);

    a = Op(b,c);

    ++m_instr_ptr;
}

template<typename TOperator>
constexpr void VirtualMachine::ExecuteUnaryOp(TOperator const& Op) noexcept
{
    Word& a = DecodeRegister(m_memory[++m_instr_ptr]);
    const Word b = GetValue(m_memory[++m_instr_ptr]);

    a = Op(b);

    ++m_instr_ptr;
}

/** halt: 0
 *      stop execution and terminate the program
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::HALT>()
{
    m_flags.Set(Flags::HALTED);
}

/** set: 1 a b
 *     set register <a> to the value of <b>
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::SET>()
{
    ExecuteUnaryOp([](Word const& b) { return b; });
}

/** push: 2 a
 *       push <a> onto the stack
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::PUSH>()
{
    const Word a = GetValue(m_memory[++m_instr_ptr]);
    StackPush(a);
    ++m_instr_ptr;
}

/** pop: 3 a
 *      remove the top element from the stack and write it into <a>; empty stack = error
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::POP>()
{
    Word& a = DecodeRegister(m_memory[++m_instr_ptr]);
    a = StackPop();
    ++m_instr_ptr;
}

/** eq: 4 a b c
 *     set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::EQ>()
{
    ExecuteBinaryOp([](Word const& b, Word const& c){ return b == c; });
}


/** gt: 5 a b c
 *     set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::GT>()
{
   ExecuteBinaryOp([](Word const& b, Word const& c){ return b > c; });
}

/** jmp: 6 a
 *      jump to <a>
 */ 
template<>
constexpr void VirtualMachine::Execute<InstructionData::JMP>()
{
    const Word A = GetValue(m_memory[++m_instr_ptr]);
    m_instr_ptr = Address(A);
}

/** jt: 7 a b
 *     if <a> is nonzero, jump to <b>
 */ 
template<>
constexpr void VirtualMachine::Execute<InstructionData::JT>()
{
    const Word A = GetValue(m_memory[++m_instr_ptr]);
    
    if(!A.is_zero())
    {
        const auto B = Address(GetValue(m_memory[++m_instr_ptr]));
        m_instr_ptr = B;
    } else {
        m_instr_ptr += 2;
    }
}

/** jf: 8 a b
 *      if <a> is zero, jump to <b>
 */ 
template<>
constexpr void VirtualMachine::Execute<InstructionData::JF>()
{
    const Word A = GetValue(m_memory[++m_instr_ptr]);
    
    if(A.is_zero())
    {
        const auto B = Address(GetValue(m_memory[++m_instr_ptr]));
        m_instr_ptr = B;
    } else {
        m_instr_ptr += 2;
    }
}

/** add: 9 a b c
 *    assign into <a> the sum of <b> and <c> (modulo 32768)
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::ADD>()
{
    ExecuteBinaryOp([](Word const& b, Word const& c){ return b + c; });
}

/** mult: 10 a b c
 *      store into <a> the product of <b> and <c> (modulo 32768)
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::MULT>()
{
    ExecuteBinaryOp([](Word const& b, Word const& c){ return b * c; });
}

/** mod: 11 a b c
 *      store into <a> the remainder of <b> divided by <c>
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::MOD>()
{
    ExecuteBinaryOp([](Word const& b, Word const& c){ return b % c; });
}

/**and: 12 a b c
 *     stores into <a> the bitwise and of <b> and <c>
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::AND>()
{
    ExecuteBinaryOp([](Word const& b, Word const& c){ return b & c; });
}

/**or: 13 a b c
 *     stores into <a> the bitwise or of <b> and <c>
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::OR>()
{
    ExecuteBinaryOp([](Word const& b, Word const& c){ return b | c; });
}

/**not: 14 a b
 *     stores 15-bit bitwise inverse of <b> in <a>
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::NOT>()
{
    ExecuteUnaryOp([](Word const& b) { return ~b; });
}

/** rmem: 15 a b
 *      read memory at address <b> and write it to <a>
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::RMEM>()
{
    Word& a = DecodeRegister(m_memory[++m_instr_ptr]);
    const auto b = Address(GetValue(m_memory[++m_instr_ptr]));

    a = m_memory[b];

    ++m_instr_ptr;
}

/** wmem: 16 a b
 *      write the value from <b> into memory at address <a>
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::WMEM>()
{
    const auto a = Address(GetValue(m_memory[++m_instr_ptr]));
    const Word b = GetValue(m_memory[++m_instr_ptr]);

    m_memory[a] = b;

    ++m_instr_ptr;
}

/** call: 17 a
 *      write the address of the next instruction to the stack and jump to <a>
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::CALL>()
{
    const auto call_destination = Address(GetValue(m_memory[++m_instr_ptr]));
    const auto return_destination = (++m_instr_ptr).get();
    StackPush(return_destination);
    m_instr_ptr = call_destination;
}

/** ret: 18
 *      remove the top element from the stack and jump to it; empty stack = halt
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::RET>()
{
    const auto return_destination = Address(StackPop());
    m_instr_ptr = return_destination;
}

/** out: 19 a
 *     write the character represented by ascii code <a> to the terminal
 */
template<>
void VirtualMachine::Execute<InstructionData::OUT>()
{
    m_output_buffer << GetValue(m_memory[++m_instr_ptr]).lo();
    ++m_instr_ptr;
}

/** in: 20 a
 *    read a character from the terminal and write its ascii code to <a>; it
 *    can be assumed that once input starts, it will continue until a newline
 *    is encountered; this means that you can safely read whole lines from the
 *    keyboard and trust that they will be fully read
 */
template<>
void VirtualMachine::Execute<InstructionData::IN>()
{
    m_input_buffer >> DecodeRegister(m_memory[++m_instr_ptr]);
    ++m_instr_ptr;
}

/** noop: 21
 *      no operation
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::NOOP>()
{
    ++m_instr_ptr;
}

/** wrong_opcode: 22 -- 0x7FFF
 *      wrong opcode or instruction not implemented
 */
template<InstructionData::OpCode TOp>
constexpr void VirtualMachine::Execute()
{
    m_flags.Set(Flags::ERROR);
    Execute<InstructionData::HALT>();
}

constexpr void VirtualMachine::ExecuteNextInstruction()
{
    switch (InstructionData::to_opcode(m_memory[m_instr_ptr]))
    {
        case InstructionData::HALT:  return Execute<InstructionData::HALT>();
        case InstructionData::SET:   return Execute<InstructionData::SET>();
        case InstructionData::PUSH:  return Execute<InstructionData::PUSH>();
        case InstructionData::POP:   return Execute<InstructionData::POP>();
        case InstructionData::EQ:    return Execute<InstructionData::EQ>();
        case InstructionData::GT:    return Execute<InstructionData::GT>();
        case InstructionData::JMP:   return Execute<InstructionData::JMP>();
        case InstructionData::JT:    return Execute<InstructionData::JT>();
        case InstructionData::JF:    return Execute<InstructionData::JF>();
        case InstructionData::ADD:   return Execute<InstructionData::ADD>();
        case InstructionData::MULT:  return Execute<InstructionData::MULT>();
        case InstructionData::MOD:   return Execute<InstructionData::MOD>();
        case InstructionData::AND:   return Execute<InstructionData::AND>();
        case InstructionData::OR:    return Execute<InstructionData::OR>();
        case InstructionData::NOT:   return Execute<InstructionData::NOT>();
        case InstructionData::RMEM:  return Execute<InstructionData::RMEM>();
        case InstructionData::WMEM:  return Execute<InstructionData::WMEM>();
        case InstructionData::CALL:  return Execute<InstructionData::CALL>();
        case InstructionData::RET:   return Execute<InstructionData::RET>();
        case InstructionData::OUT:   return Execute<InstructionData::OUT>();
        case InstructionData::IN:    return Execute<InstructionData::IN>();
        case InstructionData::NOOP:  return Execute<InstructionData::NOOP>();
        
        default:  return Execute<InstructionData::WRONG_OPCODE>();
    }
}

VirtualMachine::InputBuffer& operator>>(VirtualMachine::InputBuffer& buffer, Word& t)
{
    if(buffer.ptr == buffer.data.size())
    {
        bool no_text = true;
        while(no_text)
        {
            buffer.data.clear();
            std::getline(*buffer.m_istream, buffer.data);

            if(buffer.data == "~")
            {
                buffer.m_parent_vm.Pause();
            } else {
                no_text = false;
            }
        }
        buffer.data.push_back('\n');
        buffer.ptr = 0;
    }
    t.lo() = buffer.data[buffer.ptr++];
    t.hi() = 0;
    return buffer;
}

void VirtualMachine::Pause()
{
    std::cout << "\nExecution paused" << std::endl;

    while(true)
    {
        std::cout << "pause_console$ ";
        std::string answ;
        std::getline(std::cin, answ);


        if(answ == "debug" || answ == "d")
        {
            ToggleProcess<DebugProcess>();
        }
        else if(answ == "exit")
        {
            exit(EXIT_SUCCESS);
        }
        else if(answ == "halt")
        {
            flags().Set(Flags::HALTED | Flags::INTERRUPT);
            break;
        }
        else if(answ == "help" || answ == "h")
        {
            std::cout << "Execution pause menu. Use any of the following:\n";
            std::cout << "- debug      Enables/diables step-by-step debugging.\n";
            std::cout << "- exit       Exits the program.\n";
            std::cout << "- halt       Raises the HALT flag in the VM causing it to stop after the currently interrupted instruction is finished.\n";
            std::cout << "- help       Prints this help screen\n";
            std::cout << "- resume     Resumes execution.\n";
            std::cout << "- save       Saves the state of the VM after the currently interrupted instruction is finished.\n";
            std::cout << "- state      Dumps the contents of the VM onto the console.\n";
        }
        else if(answ == "resume" || answ == "r")
        {
            break;
        }
        else if(answ == "save" || answ == "s")
        {
            ToggleProcess<DumpContentsProcess>();
        }
        else if(answ == "state")
        {
            Print();
        }
        else
        {
            std::cout << "Unknown command. Write 'help' to see a list of available commands." << std::endl;
        }
    }
}