#include "processes.h"
#include "virtual_machine.h"
#include "serializer.h"
#include <tuple>

bool DumpContentsProcess::operator()()
{
    const std::string filename = "synacor_vm_dump.dmp";
    std::ofstream outfile(filename, std::ios_base::binary);

    Serializer::Write(outfile, m_parent_vm);

    std::cout << "Virtual machine state dumped into file: " << filename << std::endl;

    return true;
}


DebugProcess::DebugProcess(VirtualMachine & vm) : BaseProcess(vm)
{
    m_original_ostream = m_parent_vm.output_stream().m_ostream;
    m_parent_vm.output_stream().m_ostream = &m_outstream;
}

DebugProcess::~DebugProcess() noexcept
{
    m_parent_vm.output_stream().m_ostream = m_original_ostream;
}

bool DebugProcess::operator()()
{
    std::cout << "====================== STEP #" << m_step_count << "======================\n";
    m_parent_vm.Print();

    std::cout << "Output: \n" << m_outstream.str();

    std::cout << std::endl;
    std::string tmp;
    std::getline(std::cin, tmp);

    if(tmp == "~") m_parent_vm.Pause();

    std::ignore = system("clear");
    ++ m_step_count;

    return false;
}