#include "virtual_machine.h"

void VirtualMachine::LoadMemory(program_file_t& source)
{
    m_memory.load(source);
}

void VirtualMachine::Initialize()
{
    m_instr_ptr = 0;
}


void VirtualMachine::Run()
{
    while(!m_flags.Is(Flags::HALTED))
    {
        ExecuteNextInstruction();
    }
}