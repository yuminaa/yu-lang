# This file is part of the Yu programming language and is licensed under MIT License;
# See LICENSE.txt for details

add_subdirectory(ast)
add_subdirectory(interfaces)
add_subdirectory(lexer)
add_subdirectory(parser)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_library(YU_COMPILER STATIC ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)

target_link_libraries(YU_COMPILER
        PUBLIC
        YU_AST
        YU_INTERFACE
        YU_LEXER
        YU_PARSER
)
