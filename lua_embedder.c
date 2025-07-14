/**
 * Generic Lua library embedder
 * Usage: lua_embedder.exe <lua_file> <output_header> <var_name>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: %s <lua_file> <output_header> <var_name>\n", argv[0]);
        printf("Example: %s fs.lua fslib_lua.h fslib_lua\n", argv[0]);
        return 1;
    }

    const char *lua_file = argv[1];
    const char *output_header = argv[2];
    const char *var_name = argv[3];

    FILE *inf = fopen(lua_file, "rt");
    if (!inf)
    {
        printf("Error: Cannot open input file %s\n", lua_file);
        return 1;
    }

    FILE *outf = fopen(output_header, "wt");
    if (!outf)
    {
        printf("Error: Cannot create output file %s\n", output_header);
        fclose(inf);
        return 1;
    }

    // Generate header guard name from filename
    char guard_name[256];
    strcpy(guard_name, output_header);

    // Convert to uppercase and replace dots/dashes with underscores
    for (int i = 0; guard_name[i]; i++)
    {
        if (guard_name[i] >= 'a' && guard_name[i] <= 'z')
        {
            guard_name[i] = guard_name[i] - 'a' + 'A';
        }
        else if (guard_name[i] == '.' || guard_name[i] == '-')
        {
            guard_name[i] = '_';
        }
    }

    // Write header
    fprintf(outf, "#ifndef _%s_\n", guard_name);
    fprintf(outf, "#define _%s_\n\n", guard_name);
    fprintf(outf, "/* THIS IS A PROGRAMMATICALLY GENERATED FILE. DO NOT EDIT */\n\n");
    fprintf(outf, "const char *%s = \"\\\n", var_name);

    // Process lua file line by line
    char line[1024];
    while (fgets(line, sizeof(line), inf))
    {
        // Remove newline characters
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        {
            line[--len] = '\0';
        }

        // Escape special characters
        for (size_t i = 0; i < len; i++)
        {
            switch (line[i])
            {
            case '\\':
                fprintf(outf, "\\\\");
                break;
            case '"':
                fprintf(outf, "\\\"");
                break;
            default:
                fprintf(outf, "%c", line[i]);
                break;
            }
        }
        fprintf(outf, "\\r\\n\\\n");
    }

    // Write footer
    fprintf(outf, "\";\n\n");
    fprintf(outf, "#endif\n");

    fclose(inf);
    fclose(outf);

    printf("Generated %s from %s\n", output_header, lua_file);
    return 0;
}
