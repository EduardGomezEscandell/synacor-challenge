#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "virtual_machine.h"


void Help()
{
    std::cout << "          SYNACOR CHALLENGE VIRTUAL MACHINE\n";
    std::cout << "Usage:\n\n";
    std::cout << "$ vm [FLAGS...] FILENAME\n\n";
    std::cout << "Available flags:\n";
    std::cout << " --help  or -h      Prints this help message\n";
    std::cout << " --debug or -d      Runs the VM in debug mode\n";
    std::cout << std::endl;
}

void ReadArg(std::string const& arg, std::string & filename, bool & debug_mode)
{
    if(arg[0] != '-')
    {
        filename = arg;
        return;
    }

    if(arg == "-h" || arg == "--help")
    {
        Help();
        return;
    }

    if(arg == "-d" || arg == "--debug")
    {
        debug_mode = true;
        return;
    }

    std::cerr << "Unrecognized argument: " << arg <<". Use --help to see usage" << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char * argv[])
{
    bool debug_mode = false;
    std::string filename = "";

    for(int i = 1; i < argc; ++i)
    {
        ReadArg(argv[i], filename, debug_mode);
    }

    if(filename == "")
    {
        std::cerr << "A filename has not been specified!" <<" Use --help to see usage" << std::endl;
        return EXIT_FAILURE;
    }

    VirtualMachine vm;

    auto program = VirtualMachine::program_file_t(filename, std::ios::binary);

    vm.LoadMemory(program);
    
    if(debug_mode) vm.ToggleProcess<DebugProcess>();
    
    vm.Run();

    return EXIT_SUCCESS;
}