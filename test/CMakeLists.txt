add_executable(run_tests run_tests.cpp)

target_link_libraries(run_tests synacor_vm_lib doctest)
