#include "cpu.h"

void CPU::LoadMemory(std::basic_istream<raw_word_t>& source)
{
    m_memory.load(source);
}

void CPU::Initialize()
{
    m_instr_ptr = 0;
}


void CPU::Run()
{
    while(!m_flags.Is(Flags::HALTED))
    {
        ExecuteNextInstruction();
    }
}