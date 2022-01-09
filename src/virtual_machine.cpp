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

#include <functional>
#include <stdexcept>
#include <unistd.h>
#include <signal.h>


void VirtualMachine::LoadMemory(program_file_t& source)
{
    m_memory.load(source, m_stack_ptr);
}

void VirtualMachine::Run()
{
    InitializeSignalHandler();
    StackInit();

    while(!m_flags.Is(Flags::HALTED | Flags::ERROR))
    {
        ExecuteNextInstruction();
    }
}

void VirtualMachine::RunDebug()
{
    InitializeSignalHandler();
    StackInit();

    std::cout << "Running synacor VM in debug mode. Press any key after every step to continue" << std::endl;
    std::stringstream ss;
    m_ostream = &ss;

    std::size_t instr_count = 0;
    while(!m_flags.Is(Flags::HALTED | Flags::ERROR))
    {
        std::cout << "====================== STEP #" << instr_count << "======================\n";
        Print();

        std::cout << "Output: \n" << ss.str();

        ExecuteNextInstruction();
        std::cout << std::endl;

        // std::ignore = std::cin.get();
        for(std::size_t i=0; i < 50; ++i)
            std::cout << "\n";
        std::cout << std::flush; // clearing console
        ++ instr_count;
    }

    std::cout << "======================  DONE ======================\n";
    std::cout << ss.str() << std::endl;
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

    std::cout << "\nMemory around instruction pointer:\n";
    const std::size_t instr_ptr_row = m_instr_ptr.get().to_int() / 8;
    memory().hex_dump(instr_ptr_row, instr_ptr_row+2, m_instr_ptr.get().to_int());

    std::cout << "\nStack:\n";
    const std::size_t stack_base_ptr_row = m_stack_base_ptr.get().to_int() / 8;
    const std::size_t stack_ptr_row = m_stack_ptr.get().to_int() / 8;
    memory().hex_dump(stack_base_ptr_row, stack_ptr_row+1, m_stack_ptr.get().to_int());
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
    *m_ostream << GetValue(m_memory[++m_instr_ptr]).lo();
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

VirtualMachine::TextBuffer& operator>>(VirtualMachine::TextBuffer& tbuffer, Word& t)
{
    if(tbuffer.ptr == tbuffer.data.size())
    {
        tbuffer.data.clear();
        std::getline(tbuffer.m_istream, tbuffer.data);
        tbuffer.data.push_back('\n');
        tbuffer.ptr = 0;
    }
    t.lo() = tbuffer.data[tbuffer.ptr++];
    t.hi() = 0;
    return tbuffer;
}

void VirtualMachine::InitializeSignalHandler()
{
    active_vm = this;

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = sig_handle;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

void VirtualMachine::sig_handle(int sig_id)
{
    switch (sig_id) {
        case 2: break; // KEYBOARD INTERRUPT
        default:
            std::cout << "\nCaught signal " << sig_id << std::endl;
            exit(EXIT_FAILURE);
    }

    std::cout << "\nCaught signal KEYBOARD_INTERRUPT" << std::endl;

    const auto& vm = VirtualMachine::GetActiveVM();

    std::cout << ">> Current state:\n";
    vm->Print();

    constexpr auto question = [](const std::string_view question)
    {
        std::cout << question; 
        std::string a;
        while(true)
        {
            std::cin >> a;
            if(a == "y" || a=="yes") return true;
            if(a == "n" || a=="no") return false;
            std::cout << "Input not recognized. Please, answer y or n: ";
        }
    };

    if(question("Do you wish to save the current state before terminating?"))
    {
        const std::string filename = "synacor_vm_dump.dmp";
        std::ofstream outfile(filename, std::ios_base::binary);

        Serializer::Write(outfile, vm);

        std::cout << "Virtual machine state dumped into file: " << filename << std::endl;
    }

    std::cout << "Terminating" << std::endl;

    exit(EXIT_FAILURE);
}