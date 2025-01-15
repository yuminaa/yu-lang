add_library(YU_INTERFACE INTERFACE
        include/error.h
        include/tokens.h)

target_include_directories(YU_INTERFACE
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)