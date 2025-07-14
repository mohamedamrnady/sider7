#ifndef _LUALIBS_H
#define _LUALIBS_H

#include "lua.hpp"

// Structure to hold lua library information
struct lua_lib_info
{
    const char *name;        // Library name for error messages
    const char *lua_code;    // Embedded lua code
    int *lib_index;          // Pointer to store library index
    const char *table_field; // Field name in sider table (can be different from name)
};

// Generic function to load embedded lua libraries
void load_lua_library(lua_State *L, const lua_lib_info &lib_info);

// Initialize all embedded lua libraries
void init_all_lua_libs(lua_State *L);

// Assign all loaded libraries to the sider table
void assign_all_libs_to_sider_table(lua_State *L);

#endif
