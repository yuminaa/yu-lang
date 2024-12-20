# This file is part of the Yu programming language and is licensed under MIT License;
# See LICENSE.txt for details

cmake_minimum_required(VERSION 3.29)

add_executable(YU_CLI
        src/cli.cpp
        ${CMAKE_SOURCE_DIR}/yu/src/tokens.cpp
        ${CMAKE_SOURCE_DIR}/yu/include/tokens.h
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

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(YU_CLI PRIVATE c++abi)
endif ()