add_library(serial STATIC serial.c serial/serial.h)
target_include_directories(serial PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(serial PUBLIC -Wall -Wextra -Werror)

set_target_properties(serial PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

