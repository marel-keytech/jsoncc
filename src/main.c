/*
 * Copyright (c) 2015, Marel hf
 * Copyright (c) 2015, Andri Yngvason
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include "desc_parser.h"
#include "obj.h"
#include "lua_codegen.h"

#define READ_CHUNK_SIZE 256

#ifndef TEMPLATE_PATH
#define TEMPLATE_PATH "./templates"
#endif

#define C_SOURCE "c_source.lua"
#define C_HEADER "c_header.lua"

void usage()
{
    printf("\
Usage: jsoncc [options] input-file\n\
\n\
Options:\n\
    -H, --header                  Generate header.\n\
    -h, --help                    Get help.\n\
    -n, --name=NAME               Struct name (default is file name).\n\
    -s, --source                  Generate source.\n\
    -t, --template-path=PATH      Specify path to templates.\n\
");
}

struct buffer {
    char* data;
    size_t reserved;
    size_t length;
};

int buffer_grow(struct buffer* self, size_t new_size)
{
    if(self->reserved >= new_size)
        return 0;

    self->reserved += new_size * 2;
    
    self->data = realloc(self->data, self->reserved);

    return self->data != 0 ? 0 : -1;
}

char* read_input_file(const char* path)
{
    FILE* file;
    struct buffer buffer;
    size_t rsize;

    file = fopen(path, "r");
    if(!file)
        return NULL;

    memset(&buffer, 0, sizeof(buffer));

    do
    {
        if(buffer_grow(&buffer, buffer.length + READ_CHUNK_SIZE) < 0)
            goto failure;

        rsize = fread(&buffer.data[buffer.length], 1, READ_CHUNK_SIZE, file);
        buffer.length += rsize;
    }
    while(rsize == READ_CHUNK_SIZE);

    if(buffer_grow(&buffer, buffer.length + 1) < 0)
        goto failure;

    buffer.data[buffer.length++] = '\0';

    fclose(file);
    return buffer.data;

failure:
    if(file)
        fclose(file);
    if(buffer.data)
        free(buffer.data);
    return NULL;
}

char* strip_extension(char* path)
{
    char* first_dot = strchr(path, '.');
    if(first_dot)
        *first_dot = '\0';
    return path;
}

int main(int argc, char* argv[])
{
    int r = 1;
    int c;

    int is_source = 0;
    int is_header = 0;
    const char* name = NULL;
    const char* template_path = TEMPLATE_PATH;

    static const struct option options[] = {
        { "header",        no_argument,       0, 'H' },
        { "help",          no_argument,       0, 'h' },
        { "name",          required_argument, 0, 'n' },
        { "source",        no_argument,       0, 's' },
        { "template-path", required_argument, 0, 't' },
        { 0, 0, 0, 0 }
    };

    while(1)
    {
        c = getopt_long(argc, argv, "Hhn:st:", options, NULL);
        if(c == -1)
            break;

        switch(c)
        {
        case 'H':
            is_header = 1;
            break;
        case 's':
            is_source = 1;
            break;
        case 'h':
            usage();
            return 0;
        case 'n':
            name = optarg;
            break;
        case 't':
            template_path = optarg;
            break;
        default:
            usage();
            return 1;
        }
    }

    if(is_header && is_source)
    {
        fprintf(stderr, "Can't output source and header at the same time.\n");
        usage();
        return 1;
    }

    if(!is_header && !is_source)
    {
        fprintf(stderr, "Please specify either source or header output: -s or -H.\n");
        usage();
        return 1;
    }

    if(argc - optind < 1)
    {
        fprintf(stderr, "Too few arguments!\n");
        return 1;
    }

    const char* input_path = argv[optind];
    if(strcmp("-", input_path) == 0)
        input_path = "/dev/stdin";

    char* input = read_input_file(input_path);
    if(!input)
    {
        fprintf(stderr, "Failed to read input file '%s': %s\n", input_path,
                strerror(errno));
        return 1;
    }

    if(!name)
        name = strip_extension(basename(input_path));

    struct obj* obj = desc_parse(input);
    if(!obj)
    {
        desc_print_error_report();
        goto failure;
    }

    char template[256];
    snprintf(template, sizeof(template), "%s/%s", template_path,
             is_header ? C_HEADER : C_SOURCE);
    template[sizeof(template) - 1] = '\0';

    lua_codegen(template, name, obj);

    obj_free(obj);

    r = 0;
failure:
    free(input);
    return r;
}

