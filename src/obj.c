#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "obj.h"

struct obj* obj_new(int type, const char* name, size_t length)
{
    struct obj* obj = malloc(obj_sizeof(type));
    if(!obj)
        return NULL; /* TODO: Use longjmp */

    memset(obj, 0, obj_sizeof(type));

    obj->type = type;
    printf("New object: %s %s\n\n", obj_strtype(obj), name);
    obj->length = length;
    obj->name = strdup(name);
    if(!obj->name)
        goto failure;

    return obj;

failure:
    free(obj);
    return NULL; /* TODO: Use longjmp */
}

struct obj* obj_obj_new(const char* name, struct obj* children)
{
    struct obj_obj* obj = (void*)obj_new(OBJECT, name, 1);
    if(!obj)
        return NULL;

    obj->children = children;

    return (void*)obj;
}

void obj_free(struct obj* obj)
{
    if(obj->type == OBJECT)
        obj_free(obj_children(obj)); /* <- not tail recursion */

    struct obj* tail = obj->next;
    free(obj);
    if(tail)
        obj_free(tail); /* <- tail recursion */
}

const char* obj_strtype(const struct obj* obj)
{
    switch(obj->type)
    {
    case INTEGER: return "int";
    case STRING:  return "string";
    case REAL:    return "real";
    case OBJECT:  return "object";
    default:      break;
    }
    abort();
    return NULL;
}

void obj_dump(const struct obj* obj)
{
    if(obj->type == OBJECT)
    {
        printf("object {\n");
        obj_dump(obj_children((struct obj*)obj));
        printf("}.\n");
    }
    else
    {
        if(obj->length == 1)
            printf("%s %s.\n", obj_strtype(obj), obj->name);
        else
            printf("%s %s[%u].\n", obj_strtype(obj), obj->name, obj->length);
    }

    struct obj* tail = obj->next;
    if(tail)
        obj_dump(tail);
}

