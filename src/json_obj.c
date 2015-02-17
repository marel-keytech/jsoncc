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
