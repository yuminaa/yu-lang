# This file is part of the Yu programming language and is licensed under MIT License;
# See LICENSE.txt for details

set(AST_SRC
        include/nodes.h
        src/nodes.cpp
        include/expressions.h
        include/statements.h
        include/ast.h
)

add_library(YU_AST STATIC ${AST_SRC})

target_include_directories(YU_AST
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../yu/include
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_BINARY_DIR}/include
)

set_target_properties(YU_AST PROPERTIES
        CXX_STANDARD_REQUIRED ON
)
