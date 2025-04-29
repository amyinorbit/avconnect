add_library(mscrpit STATIC
    mscript/mscript.h
    mscript_impl.h
    parser.h

    parser_tokens.c
)


target_include_directories(mscript PUBLIC serial)
target_compile_options(mscript PUBLIC -Wall -Wextra -Werror)

set_target_properties(mscript PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)
