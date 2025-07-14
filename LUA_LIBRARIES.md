# Embedded Lua Libraries System

This document explains the simplified and unified system for embedding Lua libraries into the sider binary.

## Overview

The new system replaces the individual `makefslibhdr.c` and `makememlibhdr.c` programs with a single, generic `lua_embedder.c` tool that can convert any Lua file into a C header containing the Lua code as a string literal.

## Files in the System

### Core Files

- `lua_embedder.c` - Generic tool to convert .lua files to .h files
- `lualibs.h` - Header defining the library management system
- `lualibs.cpp` - Implementation of the unified library loader

### Existing Libraries

- `fs.lua` / `fslib_lua.h` - File system utilities
- `memory.lua` / `memlib_lua.h` - Memory manipulation utilities

## How to Add a New Lua Library

### Step 1: Create Your Lua Library

Create a new `.lua` file in the root directory. For example, `util.lua`:

```lua
-- util : utility functions
local m = {}

function m.split(str, delimiter)
    -- implementation
end

return m
```

### Step 2: Update the Makefile

Add rules to generate the header and compile the library:

```makefile
# Add to the embedded lua libraries section
utillib_lua.h: util.lua lua_embedder.exe
	lua_embedder.exe util.lua utillib_lua.h utillib_lua
```

### Step 3: Update lualibs.cpp

Add your library to the `libraries` array at the top of the file:

1. Add the include for your generated header:

```cpp
#include "utillib_lua.h"
```

2. Add your library to the static libraries array:

```cpp
static lua_lib_info libraries[] = {
    {"fs", fslib_lua, &_fs_lib_index, "fs"},
    {"memory", memlib_lua, &_memory_lib_index, "memory"},
    {"util", utillib_lua, &_util_lib_index, "util"},
    {"mylib", myliblib_lua, &_mylib_lib_index, "mylib"},  // <- Add this line
};
```

### Step 4: Add Library Index Variable

Add a global variable to track the library index in sider.cpp:

1. Add the variable declaration with the other lib indices:

```cpp
int _mylib_lib_index = 0;
```

2. Add the extern declaration to lualibs.cpp:

```cpp
extern int _mylib_lib_index;
```

### Step 5: Build

Run the build process:

```cmd
nmake clean
nmake
```

The library will be automatically:

- Loaded during initialization via `init_all_lua_libs()`
- Assigned to the sider table via `assign_all_libs_to_sider_table()`

## Benefits of the New System

1. **Unified Tool**: Single `lua_embedder.exe` handles all Lua-to-header conversions
2. **Easy Addition**: Adding new libraries requires minimal changes
3. **Automatic Assignment**: Libraries are automatically assigned to the sider table
4. **Consistent**: All libraries follow the same pattern
5. **Maintainable**: Less code duplication
6. **Error Handling**: Centralized error handling for library loading

## Migration from Old System

The old individual `makefslibhdr.c` and `makememlibhdr.c` files have been replaced. The generated headers maintain the same format, so existing code continues to work.

### What Changed

- `makefslibhdr.exe` and `makememlibhdr.exe` → `lua_embedder.exe`
- Individual `init_fslib()` and `init_memlib()` calls → single `init_all_lua_libs()` call
- Individual library table assignments → single `assign_all_libs_to_sider_table()` call
- Scattered library initialization → centralized in `lualibs.cpp`

### What Stayed the Same

- Generated header format is identical
- Library functionality remains unchanged
- Lua code in the .lua files is unmodified

## Example: Adding a JSON Library

Let's say you want to add a JSON parsing library:

1. Create `json.lua`:

```lua
local m = {}
function m.parse(str) -- implementation end
function m.stringify(obj) -- implementation end
return m
```

2. Update Makefile:

```makefile
jsonlib_lua.h: json.lua lua_embedder.exe
	lua_embedder.exe json.lua jsonlib_lua.h jsonlib_lua
```

3. Update lualibs.cpp:

```cpp
#include "jsonlib_lua.h"
// Add extern declaration
extern int _json_lib_index;
// Add to libraries array
{"json", jsonlib_lua, &_json_lib_index, "json"},
```

4. Add variable to sider.cpp:

```cpp
int _json_lib_index = 0;
```

5. Build and use:

```lua
local json = sider.json
local data = json.parse('{"key": "value"}')
```

Note: Libraries are accessed through the `sider` table, not via `require()`.

## Troubleshooting

### Common Issues

1. **Missing header include**: Make sure to add `#include "yourlib_lua.h"` to lualibs.cpp
2. **Library not loading**: Check that the library name in the array matches your Lua module name
3. **Build errors**: Ensure the Makefile rule for your library is correct

### Debugging

- Check the generated .h file to ensure the Lua code was embedded correctly
- Use debug output to verify library loading in the logs
- Verify the library appears in the Lua environment after loading
