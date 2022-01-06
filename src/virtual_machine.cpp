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

void VirtualMachine::Print() const
{
    std::cout << "Virtual machine\n";

    std::cout << "iPtr = " << m_instr_ptr.get().hex_dump()
              << " --> "   << m_memory[m_instr_ptr].hex_dump() << '\n';

    for(size_t i=0; i<8; ++i)
    {
        
    }
    std::cout <<" "
}