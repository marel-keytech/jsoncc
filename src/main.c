#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "parser.h"
#include "obj.h"

struct obj* lexer(char* data);

void usage()
{
    printf("Usage: jsonunpackgen <input >output\n");
}

int main(int argc, char* argv[])
{
    int r = 1;
    (void)argc;
    (void)argv;

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

    obj_dump(obj);
    obj_free(obj);

    r = 0;
failure:
    free(buffer);
    return r;
}
