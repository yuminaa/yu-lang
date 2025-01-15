# This file is part of the Yu programming language and is licensed under MIT License;
# See LICENSE.txt for details

cmake_minimum_required(VERSION 3.29)

add_executable(YU_CLI
        main.cpp
)

target_include_directories(YU_CLI PRIVATE
        include
)

set_target_properties(YU_CLI
        PROPERTIES
        CXX_STANDARD 20
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_directories(YU_COMPILER PUBLIC
            /opt/homebrew/opt/llvm/lib
    )

    target_link_libraries(YU_COMPILER
            PUBLIC
            c++
            c++abi
    )
endif()