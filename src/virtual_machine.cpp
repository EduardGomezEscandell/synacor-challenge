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
}