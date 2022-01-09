#include "serializer.h"
#include "flags.h"
#include "virtual_machine.h"

#include <algorithm>

void Serializer::Load(std::istream& in, Word & word)
{
    in >> word.lo();
    in >> word.hi();
}

void Serializer::Write(std::ostream& out, Word const& word)
{
    out << word.lo();
    out << word.hi();
}

void Serializer::Load(std::istream& in, Memory & memory)
{
    std::for_each(memory.begin(), memory.end(), [&in](Word & ptr) {Load(in, ptr); });
}

void Serializer::Write(std::ostream& out, Memory const& memory)
{
    std::for_each(memory.begin(), memory.end(), [&out](Word const& ptr) {Write(out, ptr); });
}

void Serializer::Load(std::istream& in, VirtualMachine & vm)
{
    in >> vm.m_flags.m_flags;
    std::for_each(vm.m_registers.begin(), vm.m_registers.end(), [&](Word & reg){ Load(in, reg); });
    Load(in, vm.m_instr_ptr.get());
    Load(in, vm.m_stack_base_ptr.get());
    Load(in, vm.m_stack_ptr.get());
    Load(in, vm.m_nul_register);
    Load(in, vm.m_memory);
}

void Serializer::Write(std::ostream& out, VirtualMachine const& vm)
{
    out << vm.m_flags.m_flags;
    std::for_each(vm.m_registers.begin(), vm.m_registers.end(), [&](Word const& reg){ Write(out, reg); });
    Write(out, vm.m_instr_ptr.get());
    Write(out, vm.m_stack_base_ptr.get());
    Write(out, vm.m_stack_ptr.get());
    Write(out, vm.m_nul_register);
    Write(out, vm.m_memory);
}