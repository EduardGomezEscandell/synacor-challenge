#include <cstdio>
#include <fstream>
#include "virtual_machine.h"
#include "word.h"

void Help()
{
    std::cout << "          SYNACOR CHALLENGE VIRTUAL MACHINE\n";
    std::cout << "In order to run, pass the name of the program as the only argument\n";
    std::cout << std::endl;
}

int main(int argc, char * argv[])
{
    switch(argc)
    {
        case 1: Help(); return EXIT_SUCCESS;
        case 2: break;
        default: Help(); return EXIT_FAILURE;
    }

	VirtualMachine vm;

    auto program = VirtualMachine::program_file_t(argv[1], std::ios::binary);

    vm.LoadMemory(program);

    vm.memory().hex_dump(0, 2);

    vm.Initialize();
    vm.Run();


    return EXIT_SUCCESS;
}