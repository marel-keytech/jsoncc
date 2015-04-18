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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_obj.h"

struct json_obj* json_obj_new(int type, unsigned int start, unsigned int end)
{
    struct json_obj* obj = malloc(sizeof(*obj));
    if(!obj)
        return NULL; /* TODO: Use longjmp */

    memset(obj, 0, sizeof(*obj));

    obj->type = type;
    obj->value.start = start;
    obj->value.end = end;

    return obj;
}

struct json_obj* json_obj_new_obj(int type, struct json_obj* children,
                                   unsigned int start, unsigned int end)
{
    struct json_obj* obj = json_obj_new(type, start, end);
    if(!obj)
        return NULL;

    obj->children = children;

    return obj;
}

__attribute__((visibility("default")))
void json_obj_free(struct json_obj* obj)
{
    if((obj->type == JSON_OBJ_ARRAY || obj->type == JSON_OBJ_OBJ)
       && obj->children)
    {
        json_obj_free(obj->children); /* <- not tail recursion */
    }

    struct json_obj* tail = obj->next;
    free(obj);

    if(tail)
        json_obj_free(tail); /* <- tail recursion */
}

void json_obj_set_key(struct json_obj* obj, struct json_obj_pos key)
{
    obj->key = key;
}
