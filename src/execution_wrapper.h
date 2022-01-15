#pragma once

#include "virtual_machine.h"
#include <sstream>
#include <string>

class ExecutionWrapper
{
public:

    

    static void Run(std::string const& program_path, bool debug = false)
    {
        VirtualMachine vm;

        auto program = VirtualMachine::program_file_t(program_path, std::ios::binary);

        vm.LoadMemory(program);
        
        if(debug) vm.AttachProcess<DebugProcess>();
        
        vm.Run();
    }

private:

};