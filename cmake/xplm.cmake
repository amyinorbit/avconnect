# xplm.cmake
#
# Created by Amy Parent <amy@amyparent.com>
# Copyright (c) 2024 Amy Parent
#
# Licensed under the MIT License

# Use static libc/libgcc etc
function(use_static_libc)
    if(WIN32)
        set(CMAKE_C_FLAGS
            "${CMAKE_C_FLAGS} -static-libgcc"
            PARENT_SCOPE)
        set(CMAKE_CXX_FLAGS
            "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++"
            PARENT_SCOPE)
        set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS
            "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s"
            PARENT_SCOPE)
        set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS
            "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}  -static-libgcc -static-libstdc++ -s"
            PARENT_SCOPE)
    endif()
    
endfunction(use_static_libc)

# Import acfutils libraries and create an imported target
function(find_acfutils dir)
    if(WIN32)
        set(ARCH "win64")
        set(LONG_ARCH "win-64")
    elseif(APPLE)
        set(ARCH "mac64")
        set(LONG_ARCH "mac-64")
    else()
        set(ARCH "lin64")
        set(LONG_ARCH "linux-64")
    endif()

    set(ACFLIBROOT "${dir}/qmake/${ARCH}")
    set(ACFLIB "${ACFLIBROOT}/libacfutils.a")
    
    execute_process(COMMAND ${dir}/pkg-config-deps ${LONG_ARCH} --no-link-acfutils --cflags
        OUTPUT_VARIABLE LACF_DEP_CFLAGS)
    execute_process(COMMAND ${dir}/pkg-config-deps ${LONG_ARCH} --no-link-acfutils --libs
        OUTPUT_VARIABLE LACF_DEP_LDFLAGS)
    string(REGEX REPLACE "\n$" "" LACF_DEP_LDFLAGS "${LACF_DEP_LDFLAGS}")
    string(REGEX REPLACE "\n$" "" LACF_DEP_CFLAGS "${LACF_DEP_CFLAGS}")
    
    # Does this look inane? Yes! But if we want to use target_compile_options, we need
    # something that doesn't have spaces in it, but instead a list of flags, so we
    # split the compiler flags to give it just that
    string(REPLACE " " ";" LACF_DEP_CFLAGS "${LACF_DEP_CFLAGS}")

    add_library(acfutils STATIC IMPORTED GLOBAL)
    target_include_directories(acfutils INTERFACE "${dir}/src")
    set_target_properties(acfutils PROPERTIES IMPORTED_LOCATION "${ACFLIB}")
    
    target_compile_options(acfutils INTERFACE ${LACF_DEP_CFLAGS})
    target_link_libraries(acfutils INTERFACE ${LACF_DEP_LDFLAGS} ${OPENGL_LIBRARIES})

    if(APPLE)
        target_compile_definitions(acfutils INTERFACE -DAPL=1 -DIBM=0 -DLIN=0)
    elseif(WIN32)
        target_compile_definitions(acfutils INTERFACE -DAPL=0 -DIBM=1 -DLIN=0)
    else()
        target_compile_definitions(acfutils INTERFACE -DAPL=0 -DIBM=0 -DLIN=1)
    endif()
    
    target_compile_definitions(acfutils INTERFACE
        -DCURL_STATICLIB -DGLEW_STATIC -D_LACF_WITHOUT_XPLM)
endfunction(find_acfutils)

# Import the X-Plane SDK and create targets for it
function(find_xplane_sdk SDK_ROOT)
    if(APPLE)
        find_library(XPLM_LIBRARY
            NAMES XPLM
            PATHS ${SDK_ROOT}/Libraries/Mac/
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
        )
        find_library(XPWIDGETS_LIBRARY
            XPWidgets
            ${SDK_ROOT}/Libraries/Mac/
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
        )
        find_library(CocoaLib Cocoa)
    elseif(WIN32)
        find_library(XPLM_LIBRARY
            NAMES XPLM_64
            PATHS ${SDK_ROOT}/Libraries/Win/
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
        )
        find_library(XPWIDGETS_LIBRARY
            NAMES XPWidgets_64
            PATHS ${SDK_ROOT}/Libraries/Win/
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
        )
    endif()
    
    if(WIN32)
        add_library(xplm SHARED IMPORTED GLOBAL)
        add_library(xpwidgets SHARED IMPORTED GLOBAL)
        
        set_target_properties(xplm PROPERTIES IMPORTED_IMPLIB "${XPLM_LIBRARY}")
        set_target_properties(xpwidgets PROPERTIES IMPORTED_IMPLIB "${XPWIDGETS_LIBRARY}")
        
        target_compile_definitions(xplm INTERFACE -DAPL=0 -DIBM=1 -DLIN=0)
        target_compile_definitions(xpwidgets INTERFACE -DAPL=0 -DIBM=1 -DLIN=0)
    elseif(APPLE)
        add_library(xplm SHARED IMPORTED GLOBAL)
        add_library(xpwidgets SHARED IMPORTED GLOBAL)
        
        set_target_properties(xplm PROPERTIES IMPORTED_LOCATION "${XPLM_LIBRARY}/XPLM")
        set_target_properties(xpwidgets PROPERTIES IMPORTED_LOCATION "${XPWIDGETS_LIBRARY}/XPWidgets")
        
        target_compile_definitions(xplm INTERFACE -DAPL=1 -DIBM=0 -DLIN=0)
        target_compile_definitions(xpwidgets INTERFACE -DAPL=1 -DIBM=0 -DLIN=0)
    else()
        add_library(xplm INTERFACE)
        add_library(xpwidgets INTERFACE)
        
        target_compile_definitions(xplm INTERFACE -DAPL=0 -DIBM=0 -DLIN=1)
        target_compile_definitions(xpwidgets INTERFACE -DAPL=0 -DIBM=0 -DLIN=1)
    endif()
    
    target_include_directories(xplm INTERFACE "${SDK_ROOT}/CHeaders/XPLM")
    target_include_directories(xpwidgets INTERFACE
        "${SDK_ROOT}/CHeaders/XPLM" "${SDK_ROOT}/CHeaders/Widgets")
    
    set(XPLM_INCLUDE_DIR
        "${SDK_ROOT}/CHeaders/XPLM"
        "${SDK_ROOT}/CHeaders/Widgets"
        "${SDK_ROOT}/CHeaders/Wrappers"
        CACHE INTERNAL "XPLM SDK INCLUDE DIRECTORIES"
    )
endfunction(find_xplane_sdk)

function(set_target_xplm_version lib_name SDK_VERSION)
    if(SDK_VERSION EQUAL 300)
        target_compile_definitions(${lib_name}
            PRIVATE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1)
    elseif(SDK_VERSION EQUAL 301)
        target_compile_definitions(${lib_name}
            PRIVATE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)
    elseif(SDK_VERSION EQUAL 400)
        target_compile_definitions(${lib_name}
            PRIVATE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM400=1)
    elseif(SDK_VERSION EQUAL 410)
        target_compile_definitions(${lib_name}
            PRIVATE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM400=1 -DXPLM410=1)
    else()
        message(FATAL_ERROR "Library version one of: 300, 301, 400")
    endif()
endfunction(set_target_xplm_version)

function(add_xplane_plugin lib_name SDK_VERSION ...)
    set(SRC ${ARGV})
    list(REMOVE_AT SRC 0)
    list(REMOVE_AT SRC 0)
    
    add_library(${lib_name} SHARED ${SRC})
    set_target_properties(${lib_name} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    set_target_properties(${lib_name} PROPERTIES PREFIX "")
    set_target_properties(${lib_name} PROPERTIES SUFFIX "")
    set_target_properties(${lib_name} PROPERTIES OUTPUT_NAME "${lib_name}.xpl")
    include_directories(${lib_name} PUBLIC ${XPLM_INCLUDE_DIR})
    set_target_xplm_version(${lib_name} ${SDK_VERSION})
    
    
    # Link libraries
    if(APPLE)
        target_compile_definitions(${lib_name} PUBLIC -DAPL=1 -DIBM=0 -DLIN=0)
        target_compile_definitions(${lib_name} PUBLIC -DGL_SILENCE_DEPRECATION)
        target_link_libraries(${lib_name} PUBLIC ${CocoaLib})
    	set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/mac_x64"
            RUNTIME_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/mac_x64")
    elseif(WIN32)
        target_compile_definitions(${lib_name} PUBLIC -DAPL=0 -DIBM=1 -DLIN=0)
        target_compile_definitions(${lib_name} PUBLIC -D_WIN32_WINNT=0x0600)
    	set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/win_x64"
            RUNTIME_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/win_x64"
        )
        
    else()
        target_compile_definitions(${lib_name} PUBLIC -DAPL=0 -DIBM=0 -DLIN=1)
        target_link_libraries(${lib_name} PRIVATE "-nodefaultlibs -Wl,-Bstatic -lc_nonshared -Wl,-Bdynamic")
    	set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/lin_x64"
            RUNTIME_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/lin_x64")
    endif()
    
    # target_link_libraries(${lib_name} PUBLIC xplm xpwidgets)
    
    set_target_properties(${lib_name} PROPERTIES
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
    )
    set_target_properties(${lib_name} PROPERTIES LINK_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -fno-stack-protector")
    
endfunction(add_xplane_plugin)

