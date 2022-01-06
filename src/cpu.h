#include <cstdio>
#include <iostream>
#include "instruction.h"
#include "flags.h"
#include "virtual_memory.h"

#pragma once

class CPU
{
public:
    void LoadMemory(std::basic_istream<raw_word_t>& source);
    void Initialize();
    void Run();

private:
    constexpr void ExecuteNextInstruction();

    template<InstructionData::OpCode TOp>
    constexpr void Execute();

    Flags m_flags;
    Address m_instr_ptr;
    Memory m_memory;
};


template<>
constexpr void CPU::Execute<InstructionData::HALT>()
{
    m_flags.Set(Flags::HALTED);
}

template<InstructionData::OpCode TOp>
constexpr void CPU::Execute()
{
    m_flags.Set(Flags::ERROR);
    Execute<InstructionData::HALT>();
}

template<>
constexpr void CPU::Execute<InstructionData::SET>()
{
    const auto ptr   = Address(m_memory[m_instr_ptr++]);
    const Word value = m_memory[m_instr_ptr++];
    m_memory[ptr] = value;
}

template<>
constexpr void CPU::Execute<InstructionData::ADD>()
{
    const auto p_result = Address(m_memory[m_instr_ptr++]);
    const auto p_sum1   = Address(m_memory[m_instr_ptr++]);
    const auto p_sum2   = Address(m_memory[m_instr_ptr++]);

    m_memory[p_result] = m_memory[p_sum1] + m_memory[p_sum2];
}

template<>
inline void CPU::Execute<InstructionData::OUT>()
{
    std::cout << m_memory[m_instr_ptr++];
    ++m_instr_ptr;
}

template<>
constexpr void CPU::Execute<InstructionData::NOOP>()
{
    ++m_instr_ptr;
}


constexpr void CPU::ExecuteNextInstruction()
{
    switch (InstructionData::Interpret(m_memory[m_instr_ptr]))
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
        
        default:  return Execute<InstructionData::BAD_HALT>();
    }
}