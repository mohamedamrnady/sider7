#include <windows.h>
#include "lua.hpp"
#include "sider.h"
#include "lualibs.h"

// Include generated headers
#include "fslib_lua.h"
#include "memlib_lua.h"
#include "utillib_lua.h"

// External variables for library indices
extern int _memory_lib_index;
extern int _fs_lib_index;
extern int _util_lib_index;

// Static array to hold library definitions
static lua_lib_info libraries[] = {
    {"fs", fslib_lua, &_fs_lib_index, "fs"},
    {"memory", memlib_lua, &_memory_lib_index, "memory"},
    {"util", utillib_lua, &_util_lib_index, "util"},
    // Add more libraries here as needed
    // {"mylib", mylib_lua, nullptr, "mylib"},
};

static const int num_libraries = sizeof(libraries) / sizeof(libraries[0]);

// Generic function to load embedded lua libraries
void load_lua_library(lua_State *L, const lua_lib_info &lib_info)
{
    int r = luaL_loadbuffer(L, lib_info.lua_code, strlen(lib_info.lua_code), lib_info.name);
    if (r != 0)
    {
        const char *err = lua_tostring(L, -1);
        logu_("PROBLEM loading %s library: %s. Skipping it\n", lib_info.name, err);
        lua_pop(L, 1);
        return;
    }

    if (lua_pcall(L, 0, 1, 0) != 0)
    {
        const char *err = lua_tostring(L, -1);
        logu_("PROBLEM initializing %s library: %s. Skipping it\n", lib_info.name, err);
        lua_pop(L, 1);
        return;
    }

    *(lib_info.lib_index) = lua_gettop(L);
}

// Initialize all embedded lua libraries
void init_all_lua_libs(lua_State *L)
{
    for (int i = 0; i < num_libraries; i++)
    {
        load_lua_library(L, libraries[i]);
    }
}

// Assign all loaded libraries to the sider table
void assign_all_libs_to_sider_table(lua_State *L)
{
    for (int i = 0; i < num_libraries; i++)
    {
        lua_pushvalue(L, *(libraries[i].lib_index));
        lua_setfield(L, -2, libraries[i].table_field);
    }
}
