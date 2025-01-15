# This file is part of the Yu programming language and is licensed under MIT License;
# See LICENSE.txt for details

set(PARSER_SRC
        include/parser.h
        include/symbol.h

        src/parser.cpp
        src/symbol.cpp
)

add_library(YU_PARSER STATIC ${PARSER_SRC})

target_include_directories(YU_PARSER
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../yu/include
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_BINARY_DIR}/include
)

set_target_properties(YU_PARSER PROPERTIES
        CXX_STANDARD_REQUIRED ON
)

