# This file is part of the Yu programming language and is licensed under MIT License;
# See LICENSE.txt for details

set(LEXER_SRC
        include/lexer.h
        src/lexer.cpp
)

add_library(YU_LEXER STATIC ${LEXER_SRC})

set_target_properties(YU_AST PROPERTIES
        CXX_STANDARD_REQUIRED ON
)

