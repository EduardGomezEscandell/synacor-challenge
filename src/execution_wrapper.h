#pragma once

#include "virtual_machine.h"
#include <sstream>

class ExecutionWrapper
{
public:

    struct DumpContentsProcess : public VirtualMachine::BaseProcess
    {
        DumpContentsProcess(VirtualMachine & vm) : VirtualMachine::BaseProcess(vm) { }
        bool operator()() override;
        static inline std::string name = "DumpContentsProcess";
        std::string const& Name() const noexcept override { return name; };
    };

    struct DebugProcess : public VirtualMachine::BaseProcess
    {
        DebugProcess(VirtualMachine & vm);
        bool operator()() override;
        static inline std::string name = "DebugProcess";
        std::string const& Name() const noexcept override { return name; };
    protected:
        std::stringstream m_outstream;
        std::size_t m_step_count;
    };

    static void Run(std::string const& program_path, bool debug = false)
    {
        VirtualMachine vm;

        InitializeSignalHandler(vm);

        auto program = VirtualMachine::program_file_t(program_path, std::ios::binary);

        vm.LoadMemory(program);
        
        if(debug) vm.AttachProcess<DebugProcess>();
        
        vm.Run();
    }

private:

    static void SignalHandler(int s);
    static void InitializeSignalHandler(VirtualMachine & vm);

    static inline VirtualMachine * active_machine = NULL;

};