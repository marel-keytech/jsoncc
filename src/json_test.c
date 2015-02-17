#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "json_obj.h"

struct json_obj* json_lexer(const char* data);

void usage()
{
    printf("Usage: jsontest <input >output\n");
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

    struct json_obj* obj = json_lexer(buffer);
    if(!obj)
        goto failure;

    for(struct json_obj* x = obj; x; x = x->next)
        printf("%u,%u = %u,%u\n", x->key.start, x->key.end, x->value.start, x->value.end);

    json_obj_free(obj);

    r = 0;
failure:
    free(buffer);
    return r;
}
