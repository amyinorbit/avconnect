set(PLUGIN_SRC settings.cpp str_buf.c cmd_mgr.c xplane.c)
set(PLUGIN_HDR settings.h str_buf.h cmd_mgr.h xplane.h)

set(ALL_SRC ${PLUGIN_SRC} ${PLUGIN_HDR})

add_xplane_plugin(avconnect 410 ${ALL_SRC})
add_definitions(-DDEBUG=1)
target_compile_options(avconnect PUBLIC -Wall -Wextra  -Werror)
target_link_libraries(avconnect PUBLIC serial xsb acfutils xpwidgets xplm)


add_executable(demo str_buf.c cmd_mgr.c str_buf.h cmd_mgr.h main.c)
target_compile_options(demo PUBLIC -Wall -Wextra  -Werror)
target_link_libraries(demo PUBLIC serial)
