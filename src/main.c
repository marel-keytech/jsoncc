#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "parser.h"
#include "obj.h"
#include "codegen.h"

struct obj* lexer(char* data);

void usage()
{
    printf("Usage: jsonunpackgen <input >output\n");
}

int main(int argc, char* argv[])
{
    int r = 1;
    if(argc < 2)
    {
        usage();
        return 1;
    }

    size_t size = 4096;
    char* buffer = malloc(size);
    if(!buffer)
    {
        perror("malloc()");
        return 1;
    }

    read(STDIN_FILENO, buffer, size); /* TODO: make dynamic */

    struct obj* obj = lexer(buffer);
    if(!obj)
        goto failure;

    codegen_source(argv[1], obj);

    obj_free(obj);

    r = 0;
failure:
    free(buffer);
    return r;
}
