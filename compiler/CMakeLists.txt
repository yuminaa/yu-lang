# This file is part of the Yu programming language and is licensed under MIT License;
# See LICENSE.txt for details

set(COMPILER_SRC
        lexer.h
        parser.h
        ir.h
        token.h
        builder.h
        analyzer.h

        impl/lexer.cpp
        impl/parser.cpp
        impl/token.cpp
        codegen/x86.h
        impl/builder.cpp
        impl/analyzer.cpp
        codegen/impl/x86.cpp
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
