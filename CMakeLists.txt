cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Wpedantic")

project(synacor_challenge LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(ADDRESS_SANITIZER)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

if(UB_SANITIZER)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif()

message("Compiling with: ${CMAKE_CXX_FLAGS}")

add_subdirectory(src)