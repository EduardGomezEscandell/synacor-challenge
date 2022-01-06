add_library(synacor_vm_test test_word.h)
target_link_libraries(synacor_vm_test synacor_vm_lib doctest)

set_target_properties(synacor_vm_test PROPERTIES LINKER_LANGUAGE CXX)

add_executable(run_tests run_tests.cpp)

target_link_libraries(run_tests synacor_vm_test synacor_vm_lib doctest)
