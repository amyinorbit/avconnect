add_library(ImGui
STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
)

target_compile_options(ImGui PRIVATE -Wall -Wextra  -Werror)
target_link_libraries(ImGui PUBLIC m)
target_include_directories(ImGui PUBLIC imgui)

set_target_properties(ImGui
PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fno-stack-protector"
)

add_library(xsb
STATIC
    xsb_public/ImgFontAtlas.cpp
    xsb_public/ImgWindow.cpp
)

target_compile_options(xsb PRIVATE -Wall)
target_link_libraries(xsb PUBLIC m ImGui acfutils xplm xpwidgets)
target_include_directories(xsb PUBLIC xsb_public)

set_target_properties(xsb
PROPERTIES
     C_VISIBILITY_PRESET hidden
     VISIBILITY_INLINES_HIDDEN ON
     POSITION_INDEPENDENT_CODE ON
     LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fno-stack-protector"
 )
set_target_xplm_version(xsb 301)
target_compile_definitions(xsb PUBLIC -DDEBUG=1)

add_library(serial STATIC serial/serial.c serial/serial/serial.h)

if(APPLE)
    target_sources(serial PRIVATE serial/enum_macos.c)
    target_link_libraries(serial PRIVATE $<LINK_LIBRARY:FRAMEWORK,CoreFoundation> $<LINK_LIBRARY:FRAMEWORK,IOKit>)
endif()

target_include_directories(serial PUBLIC serial)
target_compile_options(serial PUBLIC -Wall -Wextra -Werror)

set_target_properties(serial PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)


add_library(toml STATIC tomlc99/toml.h tomlc99/toml.c)
target_compile_options(toml PUBLIC -Wall)
target_include_directories(toml PUBLIC tomlc99)

set_target_properties(toml PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

# add_subdirectory(mscript)
