set(PLUGIN_SRC
    utils/str_buf.c
    utils/cmd_mgr.c
    device.c
    device_input.c
    settings.cpp
    xplane.c)
set(PLUGIN_HDR
    utils/str_buf.h
    utils/cmd_mgr.h
    utils/buffers.h
    device.h
    device_impl.h
    settings.h
    xplane.h)

set(ALL_SRC ${PLUGIN_SRC} ${PLUGIN_HDR})

add_xplane_plugin(avconnect 410 ${ALL_SRC})
add_definitions(-DDEBUG=1)
target_compile_options(avconnect PUBLIC -Wall -Wextra  -Werror)
target_link_libraries(avconnect PUBLIC serial xsb acfutils xpwidgets xplm)


add_executable(demo utils/str_buf.c utils/cmd_mgr.c utils/str_buf.h utils/cmd_mgr.h main.c)
target_compile_options(demo PUBLIC -Wall -Wextra  -Werror)
target_link_libraries(demo PUBLIC serial)
