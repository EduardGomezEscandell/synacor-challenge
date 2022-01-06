#include "cpu.h"

void CPU::Initialize()
{
	m_instr_ptr = 0;
}


void CPU::Run()
{
	while(!m_flags.Is(Flags::HALTED))
	{
		

		++m_instr_ptr;
	}
}