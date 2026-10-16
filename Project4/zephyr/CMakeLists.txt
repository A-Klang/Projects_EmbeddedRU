# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(project4_canopen LANGUAGES C CXX)

target_sources(app PRIVATE
    src/main.cpp
    src/canopen.cpp
    src/state_machine.cpp
    src/zephyr_can_adapter.cpp
    src/motor_application.cpp
    ../lib/controllers/controllers.cpp
)

target_include_directories(app PRIVATE
    include
    ../lib/controllers
)

target_compile_options(app PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
)
