# This file is part of the Yu programming language and is licensed under MIT License;
# See LICENSE.txt for details

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/googletest ${CMAKE_BINARY_DIR}/googletest)

add_executable(YU_TEST
        unittest/tokenizing.cpp
)

target_include_directories(YU_TEST PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/frontend/include
        ${CMAKE_SOURCE_DIR}/vendor/googletest/googletest/include
)

if (APPLE)
    target_link_directories(YU_TEST PRIVATE
            /opt/homebrew/opt/llvm/lib
    )
endif()

add_dependencies(YU_TEST YU_COMPILER)

target_link_libraries(YU_TEST PRIVATE
        YU_COMPILER
        GTest::gtest
        GTest::gtest_main
)

if (APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(YU_TEST PRIVATE
            c++
            c++abi
    )
endif ()

set_target_properties(YU_TEST PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(YU_TEST
        PROPERTIES
        ENVIRONMENT "GTEST_COLOR=yes"
)