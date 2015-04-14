#ifndef OBJ_H_INCLUDED_
#define OBJ_H_INCLUDED_

#include <string.h>

enum obj_type {
    OBJ_UNINITIALIZED = 0,
    OBJ_INTEGER,
    OBJ_STRING,
    OBJ_REAL,
    OBJ_OBJECT,
    OBJ_BOOL,
    OBJ_ANY
};

struct obj {
    struct obj* next;
    struct obj* children;

    enum obj_type type;
    char name[256];
    size_t length;
    int is_optional;
};

struct obj_state {
    struct obj* obj;
    int line;
};

static inline void obj_make_optional(struct obj* obj)
{
    obj->is_optional = 1;
}

static inline void obj_set_name(struct obj* obj, const char* name)
{
    strncpy(obj->name, name, sizeof(obj->name) - 1);
    obj->name[sizeof(obj->name) - 1] = 0;
}

struct obj* obj_new();
void obj_free(struct obj* obj);
const char* obj_strtype(const struct obj* obj);
const char* obj_strctype(const struct obj* obj);
void obj_dump(const struct obj* obj);

#endif /* OBJ_H_INCLUDED_ */

