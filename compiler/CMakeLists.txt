# This file is part of the Yu programming language and is licensed under MIT License;
# See LICENSE.txt for details

set(COMPILER_SRC
        include/lexer.h
        include/parser.h
        include/token.h

        src/lexer.cpp
        src/parser.cpp
        src/token.cpp

        ../common/styles.h
        ../common/arch.hpp
)

add_library(YU_COMPILER STATIC ${COMPILER_SRC})

target_include_directories(YU_COMPILER
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../yu/include
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_BINARY_DIR}/include
)

set_target_properties(YU_COMPILER PROPERTIES
        CXX_STANDARD_REQUIRED ON
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
endif ()
