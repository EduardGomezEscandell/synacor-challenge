add_library(synacor_vm_lib  address.h
                            flags.h
                            instruction.h
                            virtual_machine.h
                            virtual_machine.cpp
                            virtual_memory.h
                            word.h)

set_target_properties(synacor_vm_lib PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(synacor_vm_lib INTERFACE .)

add_executable(synacor_vm main.cpp)
target_link_libraries(synacor_vm synacor_vm_lib)