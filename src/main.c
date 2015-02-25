#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "parser.h"
#include "obj.h"
#include "lua_codegen.h"

struct obj* lexer(char* data);

enum output_type { OUTPUT_HEADER, OUTPUT_SOURCE };

void usage()
{
    printf("Usage: jsonunpackgen h|s name <input >output\n");
}

int main(int argc, char* argv[])
{
    int r = 1;
    if(argc < 3)
    {
        usage();
        return 1;
    }

    const char* typearg = argv[1];
    const char* name = argv[2];

    enum output_type output_type;

    switch(*typearg)
    {
    case 'h': output_type = OUTPUT_HEADER; break;
    case 's': output_type = OUTPUT_SOURCE; break;
    default:  usage(); return 1;
    }

    size_t size = 4096;
    char* buffer = malloc(size);
    if(!buffer)
    {
        perror("malloc()");
        return 1;
    }

    if(read(STDIN_FILENO, buffer, size) < 0) /* TODO: make dynamic */
        goto failure;

    struct obj* obj = lexer(buffer);
    if(!obj)
        goto failure;

    switch(output_type)
    {
    case OUTPUT_HEADER: lua_codegen("./templates/c_header.lua", name, obj); break;
    case OUTPUT_SOURCE: lua_codegen("./templates/c_source.lua", name, obj); break;
    default: abort(); break;
    }

    obj_free(obj);

    r = 0;
failure:
    free(buffer);
    return r;
}

