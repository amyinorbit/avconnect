add_executable(demo main.c str_buf.c str_buf.h cmd_mgr.c cmd_mgr.h)
target_link_libraries(demo PRIVATE serial)
