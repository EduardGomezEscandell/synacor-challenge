#include <iostream>
#include "instruction.h"
#include "flags.h"

#pragma once

class CPU
{
public:

    template<InstructionData::OpCode TOp>
    constexpr void Execute(
        Word & a,
        Word const& b,
        Word const& c);

    void Initialize();
    void Run();

private:
    Flags m_flags;
    Word m_aux_register;
    Word m_instr_ptr;
};


template<>
constexpr void CPU::Execute<InstructionData::HALT>(
    Word &,
    Word const&,
    Word const&)
{
    m_flags.Set(Flags::HALTED);
}

template<>
constexpr void CPU::Execute<InstructionData::OUT>(
    Word & a,
    Word const&,
    Word const&)
{
    m_aux_register = a;
    m_flags.Set(Flags::PRINT);
}

template<>
constexpr void CPU::Execute<InstructionData::NOOP>(
    Word &,
    Word const&,
    Word const&)
{
}
