# This file is part of the Yu programming language and is licensed under MIT License;
# See LICENSE.txt for details

cmake_minimum_required(VERSION 3.29)

add_executable(YU_CLI
        impl/cli.cpp
        timer.h
        style.h
        impl/timer.cpp
)

target_include_directories(YU_CLI PRIVATE
        include
)

set_target_properties(YU_CLI
        PROPERTIES
        CXX_STANDARD 20
)

target_link_libraries(YU_CLI PRIVATE
        stdc++
        YU_COMPILER
)