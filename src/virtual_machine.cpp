#include "virtual_machine.h"
#include "address.h"
#include "instruction.h"
#include "word.h"

#include <iostream>
#include <sstream>

void VirtualMachine::LoadMemory(program_file_t& source)
{
    m_memory.load(source, m_stack_ptr);
}

void VirtualMachine::Run()
{
    InitializeStack();

    while(!m_flags.Is(Flags::HALTED))
    {
        ExecuteNextInstruction();
    }
}

void VirtualMachine::RunDebug()
{
    InitializeStack();

    std::cout << "Running synacor VM in debug mode. Press any key after every step to continue" << std::endl;
    std::stringstream ss;
    m_ostream = &ss;

    std::size_t instr_count = 0;
    while(!m_flags.Is(Flags::HALTED))
    {
        std::cout << "====================== STEP #" << instr_count << "======================\n";
        Print();

        std::cout << "Output: \n" << ss.str();

        ExecuteNextInstruction();
        std::cout << std::endl;

        // std::ignore = std::cin.get();
        // for(std::size_t i=0; i < 50; ++i)
            std::cout << "\n";
        std::cout << std::flush; // clearing console
        ++ instr_count;
    }
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
    std::cout << "- HALTED: " << m_flags.Is(Flags::HALTED) << '\n';
    std::cout << "- ERROR : " << m_flags.Is(Flags::ERROR) << '\n';

    std::cout << "\nMemory around instruction pointer:\n";
    const std::size_t instr_ptr_row = m_instr_ptr.get().to_int() / 8;
    memory().hex_dump(instr_ptr_row, instr_ptr_row+2, m_instr_ptr.get().to_int());

    std::cout << "\nStack:\n";
    const std::size_t stack_base_ptr_row = m_stack_base_ptr.get().to_int() / 8;
    const std::size_t stack_ptr_row = m_stack_ptr.get().to_int() / 8;
    memory().hex_dump(stack_base_ptr_row, stack_ptr_row+1, m_stack_ptr.get().to_int());
    std::cout << std::endl;
}

constexpr Word const& VirtualMachine::GetValue(Word const& arg) const
{
    if(InstructionData::to_wordtype(arg) == InstructionData::REGISTER)
    {
        return DecodeRegister(arg);
    } else {
        return arg;
    }
}

constexpr Word& VirtualMachine::DecodeRegister(Word w)
{
#ifndef DNDEBUG
    assert(InstructionData::to_wordtype(w) == InstructionData::REGISTER);
#endif
    return m_registers[w.lo()];
}

constexpr Word const& VirtualMachine::DecodeRegister(Word w) const
{
#ifndef DNDEBUG
    assert(InstructionData::to_wordtype(w) == InstructionData::REGISTER);
#endif
    return m_registers[w.lo()];
}

constexpr void VirtualMachine::InitializeStack() noexcept
{
    m_stack_base_ptr = Address((m_stack_ptr.get().to_int() / 8 + 1) * 8); // Starts at next line
    m_stack_ptr = m_stack_base_ptr;
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

template<>
constexpr void VirtualMachine::Execute<InstructionData::HALT>()
{
    m_flags.Set(Flags::HALTED);
}

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
    m_memory[m_stack_ptr++] = a;
    ++m_instr_ptr;
}

/** pop: 3 a
 *      remove the top element from the stack and write it into <a>; empty stack = error
 */
template<>
constexpr void VirtualMachine::Execute<InstructionData::POP>()
{
    Word& a = DecodeRegister(m_memory[++m_instr_ptr]);
    a = m_memory[--m_stack_ptr];
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

template<>
constexpr void VirtualMachine::Execute<InstructionData::ADD>()
{
    ExecuteBinaryOp([](Word const& b, Word const& c){ return b + c; });
}

template<>
void VirtualMachine::Execute<InstructionData::OUT>()
{
    *m_ostream << GetValue(m_memory[++m_instr_ptr]).lo();
    ++m_instr_ptr;
}

template<>
void VirtualMachine::Execute<InstructionData::IN>()
{
    const auto val = []()
    {
        raw_word_t x = 0;
        std::cin >> x;
        return x;
    }();

    if(val > Word::max_word)
    {
        m_flags.Set(Flags::ERROR);
        m_flags.Set(Flags::BAD_INPUT);
        Execute<InstructionData::HALT>();
        return;
    }

    DecodeRegister(m_memory[++m_instr_ptr]).set_raw(val);
    ++m_instr_ptr;
}

template<>
constexpr void VirtualMachine::Execute<InstructionData::NOOP>()
{
    ++m_instr_ptr;
}

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
        
        default:  return Execute<InstructionData::FATAL_ERROR>();
    }
}