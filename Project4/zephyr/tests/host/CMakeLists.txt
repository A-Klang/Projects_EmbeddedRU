cmake_minimum_required(VERSION 3.20)
project(project4_canopen_host_tests LANGUAGES CXX)

add_executable(canopen_tests
    test_canopen.cpp
    ../../src/canopen.cpp
    ../../src/state_machine.cpp
)

target_include_directories(canopen_tests PRIVATE ../../include)
target_compile_features(canopen_tests PRIVATE cxx_std_17)
target_compile_options(canopen_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)

enable_testing()
add_test(NAME canopen_tests COMMAND canopen_tests)
