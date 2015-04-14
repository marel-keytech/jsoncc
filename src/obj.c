#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jslex.h"
#include "obj.h"

struct obj* obj_new()
{
    struct obj* self = malloc(sizeof(*self));
    if(!self)
        return NULL;

    memset(self, 0, sizeof(*self));

    return self;
}

void obj_free(struct obj* self)
{
    if(self->type == OBJ_OBJECT)
        obj_free(self->children); /* <- not tail recursion */

    struct obj* tail = self->next;
    free(self);

    if(tail)
        obj_free(tail); /* <- tail recursion */
}

const char* obj_strtype(const struct obj* obj)
{
    switch(obj->type)
    {
    case OBJ_INTEGER: return "int";
    case OBJ_STRING:  return "string";
    case OBJ_REAL:    return "real";
    case OBJ_OBJECT:  return "object";
    case OBJ_BOOL:    return "bool";
    case OBJ_ANY:     return "any";
    default:          break;
    }
    abort();
    return NULL;
}

const char* obj_strctype(const struct obj* obj)
{
    switch(obj->type)
    {
    case OBJ_INTEGER: return "long long";
    case OBJ_STRING:  return "char*";
    case OBJ_REAL:    return "double";
    case OBJ_OBJECT:  return "struct obj*";
    case OBJ_BOOL:    return "int";
    default:          break;
    }
    abort();
    return NULL;
}

void obj_dump(const struct obj* obj)
{
    if(obj->type == OBJ_OBJECT)
    {
        printf("object {\n");
        obj_dump(obj->children);
        printf("}.\n");
    }
    else
    {
        if(obj->length == 1)
            printf("%s %s.\n", obj_strtype(obj), obj->name);
        else
            printf("%s %s[%u].\n", obj_strtype(obj), obj->name, obj->length);
    }

    if(obj->name)
        obj_dump(obj->next);
}

