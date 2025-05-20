cmake_minimum_required(VERSION 3.15)
project(sider7 LANGUAGES C CXX ASM)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(DEBUG_BUILD "Build in debug mode" OFF)
option(PERF_TESTING "Enable performance testing" OFF)

# Compiler flags
if(MSVC)
    # Set the exact compiler flags from the Makefile
    # Using /MT for static release runtime to match FW1FontWrapper
    set(CMAKE_CXX_FLAGS "/nologo /Od /EHsc /wd4731 /MT /D_WIN32_WINNT=0x601")
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
    
    # Make sure CMAKE_MSVC_RUNTIME_LIBRARY is set consistently
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    
    # Debug iterator level needs to be 0
    add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)
    
    # Set the exact linker flags from the Makefile
    set(CMAKE_EXE_LINKER_FLAGS "/NOLOGO /DEFAULTLIB:\"LIBCMT\"")
    set(CMAKE_SHARED_LINKER_FLAGS "/NOLOGO /DEFAULTLIB:\"LIBCMT\"")
    set(CMAKE_MODULE_LINKER_FLAGS "/NOLOGO /DEFAULTLIB:\"LIBCMT\"")
    
    # Add debug/release flags as needed
    if(DEBUG_BUILD)
        add_definitions(-DDEBUG)
    else()
        add_definitions(-DMYDLL_RELEASE_BUILD)
    endif()
    
    if(PERF_TESTING)
        add_definitions(-DPERF_TESTING)
    endif()
endif()

# Define paths for external libraries
set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/soft/zlib)
set(LUAJIT_DIR ${CMAKE_SOURCE_DIR}/soft/luajit)
set(FW1_DIR ${CMAKE_SOURCE_DIR}/soft/FW1FontWrapper)
set(MINIAUDIO_DIR ${CMAKE_SOURCE_DIR}/soft/miniaudio)

# Include directories
include_directories(
    ${ZLIB_DIR}
    ${LUAJIT_DIR}/src
    ${FW1_DIR}/Source
    ${MINIAUDIO_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

# Libraries to link
set(LIBS 
    user32 gdi32 comctl32 version ole32 shell32 psapi
)

# Find or build external dependencies
# ZLIB
add_custom_target(zlib
    COMMAND nmake -f win32/Makefile.msc
    WORKING_DIRECTORY ${ZLIB_DIR}
)
set(ZLIB_LIBRARIES ${ZLIB_DIR}/zlib.lib)

# LuaJIT
add_custom_target(luajit
    COMMAND msvcbuild.bat gc64 static
    WORKING_DIRECTORY ${LUAJIT_DIR}/src
)
set(LUAJIT_LIBRARIES ${LUAJIT_DIR}/src/lua51.lib)

# FW1FontWrapper
add_custom_target(fw1fontWrapper
    COMMAND nmake -f ../Makefile
    COMMAND cd .. && msbuild /p:Configuration=Release
    WORKING_DIRECTORY ${FW1_DIR}/Source
)
set(FW1_LIBRARIES ${FW1_DIR}/Release/x64/FW1FontWrapper.lib)

# Generate shader headers with FXC - Fix output paths to go to build directory
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/vshader.h
    COMMAND fxc /E siderVS /Ges /T vs_4_0 /Fh ${CMAKE_BINARY_DIR}/vshader.h vshader.hlsl
    DEPENDS vshader.hlsl
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/vtexshader.h
    COMMAND fxc /E siderTexVS /Ges /T vs_4_0 /Fh ${CMAKE_BINARY_DIR}/vtexshader.h vtexshader.hlsl
    DEPENDS vtexshader.hlsl
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/pshader.h
    COMMAND fxc /E siderPS /Ges /T ps_4_0 /Fh ${CMAKE_BINARY_DIR}/pshader.h pshader.hlsl
    DEPENDS pshader.hlsl
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/ptexshader.h
    COMMAND fxc /E siderTexPS /Ges /T ps_4_0 /Fh ${CMAKE_BINARY_DIR}/ptexshader.h ptexshader.hlsl
    DEPENDS ptexshader.hlsl
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Generate memlib_lua.h
add_executable(makememlibhdr makememlibhdr.c)
# Use the same simple compilation approach as in the Makefile
if(MSVC)
    # Override compile flags to match the original Makefile behavior
    set_target_properties(makememlibhdr PROPERTIES 
        COMPILE_FLAGS "${CMAKE_C_FLAGS}"
        LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
endif()
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/memlib_lua.h
    COMMAND makememlibhdr > ${CMAKE_BINARY_DIR}/memlib_lua.h
    DEPENDS makememlibhdr memory.lua
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Generate fslib_lua.h
add_executable(makefslibhdr makefslibhdr.c)
# Use the same simple compilation approach as in the Makefile
if(MSVC)
    # Override compile flags to match the original Makefile behavior
    set_target_properties(makefslibhdr PROPERTIES 
        COMPILE_FLAGS "${CMAKE_C_FLAGS}"
        LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
endif()
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/fslib_lua.h
    COMMAND makefslibhdr > ${CMAKE_BINARY_DIR}/fslib_lua.h
    DEPENDS makefslibhdr fs.lua
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Assembly file
if(MSVC)
    enable_language(ASM_MASM)
    set(CMAKE_ASM_MASM_FLAGS "/c")
    set(ASM_SOURCES util.asm)
else()
    message(FATAL_ERROR "Only MSVC with MASM supported")
endif()

# All source files for the DLL
set(SIDER_DLL_SOURCES
    sider.cpp
    common.cpp
    kmp.cpp
    imageutil.cpp
    version.cpp
    libz.cpp
    kitinfo.cpp
    audio.cpp
    memlib.cpp
    fslib.cpp
    regs.cpp
    DDSTextureLoader.cpp
    WICTextureLoader.cpp
    ${ASM_SOURCES}
)

# Resource files
set(SIDER_DLL_RC sider.rc)
set(SIDER_EXE_RC sider_main.rc)

# Add resource compilation
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/sider.res
    COMMAND rc -r -fo ${CMAKE_BINARY_DIR}/sider.res ${CMAKE_SOURCE_DIR}/sider.rc
    DEPENDS sider.rc
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/sider_main.res
    COMMAND rc -r -fo ${CMAKE_BINARY_DIR}/sider_main.res ${CMAKE_SOURCE_DIR}/sider_main.rc
    DEPENDS sider_main.rc sider.ico
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Sider DLL
add_library(sider SHARED 
    ${SIDER_DLL_SOURCES} 
    ${CMAKE_BINARY_DIR}/sider.res
    ${CMAKE_BINARY_DIR}/vshader.h
    ${CMAKE_BINARY_DIR}/vtexshader.h
    ${CMAKE_BINARY_DIR}/pshader.h
    ${CMAKE_BINARY_DIR}/ptexshader.h
    ${CMAKE_BINARY_DIR}/memlib_lua.h
    ${CMAKE_BINARY_DIR}/fslib_lua.h
)

# Sider EXE
add_executable(sider_exe main.cpp ${CMAKE_BINARY_DIR}/sider_main.res)
set_target_properties(sider_exe PROPERTIES 
    OUTPUT_NAME "sider"
    LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

# Link sider_exe with the sider library
target_link_libraries(sider_exe 
    sider
    ${LIBS}
)

# Make sure sider_exe depends on sider
add_dependencies(sider_exe sider)

add_dependencies(sider zlib luajit fw1fontWrapper)

target_link_libraries(sider 
    ${LIBS}
    ${ZLIB_LIBRARIES} 
    ${LUAJIT_LIBRARIES}
    ${FW1_LIBRARIES}
)

# Simple playback
add_executable(simple_playback simple_playback.c)
if(MSVC)
    set_target_properties(simple_playback PROPERTIES 
        COMPILE_FLAGS "${CMAKE_C_FLAGS}"
        LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
endif()

# Patcher
add_executable(patcher patcher.cpp)
if(MSVC)
    set_target_properties(patcher PROPERTIES 
        COMPILE_FLAGS "${CMAKE_CXX_FLAGS}"
        LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
endif()

# Custom target to copy luajit.exe
add_custom_target(copy_luajit
    COMMAND ${CMAKE_COMMAND} -E copy ${LUAJIT_DIR}/src/luajit.exe ${CMAKE_BINARY_DIR}/luajit.exe
    DEPENDS luajit
)

# Main target dependencies
add_custom_target(all_targets DEPENDS sider_exe sider simple_playback patcher copy_luajit)

# Clean target
add_custom_target(cleanall
    COMMAND ${CMAKE_COMMAND} -E remove "*.obj" "*.dll" "*.exp" "*.res" "*.lib" "*.exe" "*~" "memlib_lua.h" "vshader.h" "vtexshader.h" "pshader.h" "ptexshader.h"
    COMMAND cd ${LUAJIT_DIR}/src && del /Q lua51.exp lua51.lib lua51.dll luajit.exe
    COMMAND cd ${FW1_DIR} && del /Q /S Debug Release
    COMMAND cd ${FW1_DIR}/Source && del /Q /S vsimple.h vclip.h vempty.h gsimple.h gclip.h psimple.h pclip.h
    COMMAND cd ${ZLIB_DIR} && nmake -f win32/Makefile.msc clean
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)