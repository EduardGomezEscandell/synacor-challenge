#include <cstdio>
#include <iostream>
#include "address.h"
#include "instruction.h"
#include "flags.h"
#include "virtual_memory.h"

#pragma once

class VirtualMachine
{
public:
    using program_file_t = Memory::program_file_t;

    void LoadMemory(program_file_t& source);
    void Initialize();
    void Run();

    constexpr Memory const& memory() const noexcept {return m_memory; }
    void Print() const;

private:
    constexpr void ExecuteNextInstruction();

    constexpr Word& RegisterFromInteger(Word w)
    {
#ifndef DNDEBUG
        assert(InstructionData::to_wordtype(w) == InstructionData::REGISTER);
#endif
        return m_registers[w.lo()];
    }

    constexpr Word const& RegisterFromInteger(Word w) const
    {
#ifndef DNDEBUG
        assert(InstructionData::to_wordtype(w) == InstructionData::REGISTER);
#endif
        return m_registers[w.lo()];
    }

    constexpr Word const& GetValue(Word const& arg) const
    {
        if(InstructionData::to_wordtype(arg) == InstructionData::REGISTER)
        {
            return RegisterFromInteger(arg);
        } else {
            return arg;
        }
    }

    template<InstructionData::OpCode TOp>
    constexpr void Execute();

    static constexpr std::size_t num_registers = InstructionData::num_registers;

    Flags m_flags;
    Address m_instr_ptr;
    Memory m_memory;
    Word m_registers[num_registers];
};


template<>
constexpr void VirtualMachine::Execute<InstructionData::HALT>()
{
    m_flags.Set(Flags::HALTED);
}

template<InstructionData::OpCode TOp>
constexpr void VirtualMachine::Execute()
{
    m_flags.Set(Flags::ERROR);
    Execute<InstructionData::HALT>();
}

template<>
constexpr void VirtualMachine::Execute<InstructionData::SET>()
{
    Word& r = RegisterFromInteger(m_memory[++m_instr_ptr]);
    r = GetValue(m_memory[++m_instr_ptr]);
    
    ++m_instr_ptr;
}

template<>
constexpr void VirtualMachine::Execute<InstructionData::ADD>()
{
    Word& r = RegisterFromInteger(m_memory[++m_instr_ptr]);
    const Word arg1   = GetValue(m_memory[++m_instr_ptr]);
    const Word arg2   = GetValue(m_memory[++m_instr_ptr]);

    r = arg1 + arg2;

    ++m_instr_ptr;
}

template<>
inline void VirtualMachine::Execute<InstructionData::OUT>()
{
    std::cout << GetValue(m_memory[++m_instr_ptr]).lo();
    ++m_instr_ptr;
}

template<>
constexpr void VirtualMachine::Execute<InstructionData::IN>()
{
    std::cin >> RegisterFromInteger(m_memory[++m_instr_ptr]);
    ++m_instr_ptr;
}

template<>
constexpr void VirtualMachine::Execute<InstructionData::NOOP>()
{
    ++m_instr_ptr;
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