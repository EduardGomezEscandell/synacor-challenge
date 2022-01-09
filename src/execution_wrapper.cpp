#include "execution_wrapper.h"
#include "virtual_machine.h"
#include "serializer.h"
#include "flags.h"

#include <unistd.h>
#include <signal.h>


void ExecutionWrapper::InitializeSignalHandler(VirtualMachine & vm)
{
    active_machine = &vm;

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = SignalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

bool ExecutionWrapper::DumpContentsProcess::operator()()
{
    const std::string filename = "synacor_vm_dump.dmp";
    std::ofstream outfile(filename, std::ios_base::binary);

    Serializer::Write(outfile, m_parent_vm);

    std::cout << "Virtual machine state dumped into file: " << filename << std::endl;

    return true;
}

ExecutionWrapper::DebugProcess::DebugProcess(VirtualMachine & vm) : VirtualMachine::BaseProcess(vm)
{
    vm.m_ostream = &m_outstream;
}

bool ExecutionWrapper::DebugProcess::operator()()
{
    std::cout << "====================== STEP #" << m_step_count << "======================\n";
    m_parent_vm.Print();

    std::cout << "Output: \n" << m_outstream.str();

    std::cout << std::endl;
    usleep(1);
    std::ignore = system("clear");
    ++ m_step_count;

    return false;
}

void ExecutionWrapper::SignalHandler(int sig_id)
{
    if(active_machine->paused)
    {
        std::cerr << "SYSTEM SIGNAL " << sig_id << " during pause menu. Aborting";
        exit(EXIT_FAILURE);
    }

    active_machine->paused = true;

    switch (sig_id) {
        case 2: break; // KEYBOARD INTERRUPT
        default:
            std::cout << "\nCaught signal " << sig_id << std::endl;
            exit(EXIT_FAILURE);
    }

    std::cout << "\nExecution paused" << std::endl;

    while(true)
    {
        std::cout << "pause_console$ ";
        std::string answ;
        std::getline(std::cin, answ);


        if(answ == "debug" || answ == "d")
        {
            active_machine->ToggleProcess<DebugProcess>();
        }
        else if(answ == "exit")
        {
            exit(EXIT_SUCCESS);
        }
        else if(answ == "halt")
        {
            active_machine->m_flags.Set(Flags::HALTED | Flags::INTERRUPT);
            break;
        }
        else if(answ == "help" || answ == "h")
        {
            std::cout << "Execution pause menu. Use any of the following:\n";
            std::cout << "- debug      Enables/diables step-by-step debugging.\n";
            std::cout << "- exit       Exits the program.\n";
            std::cout << "- halt       Raises the HALT flag in the VM causing it to stop after the currently interrupted instruction is finished.\n";
            std::cout << "- help       Prints this help screen\n";
            std::cout << "- resume     Resumes execution.\n";
            std::cout << "- save       Saves the state of the VM after the currently interrupted instruction is finished.\n";
            std::cout << "- state      Dumps the contents of the VM onto the console.\n";
        }
        else if(answ == "resume" || answ == "r")
        {
            break;
        }
        else if(answ == "save" || answ == "s")
        {
            active_machine->AttachProcess<DumpContentsProcess>();
        }
        else if(answ == "state")
        {
            active_machine->Print();
        }
        else
        {
            std::cout << "Unknown command. Write 'help' to see a list of available commands." << std::endl;
        }
    }

    active_machine->paused = false;
}