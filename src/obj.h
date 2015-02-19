#ifndef OBJ_H_INCLUDED_
#define OBJ_H_INCLUDED_

#include "parser.h"

struct obj {
    struct obj* next;
    int type;
    char* name;
    size_t length;
    int is_optional;
};

struct obj_obj {
    struct obj obj;
    struct obj* children;
};

struct obj_state {
    struct obj* obj;
    int line;
};

static inline size_t obj_sizeof(int type)
{
    return type == OBJECT ? sizeof(struct obj_obj) : sizeof(struct obj);
}

static inline struct obj* obj_children(struct obj* obj)
{
    return ((struct obj_obj*)obj)->children;
}

static inline void obj_make_optional(struct obj* obj)
{
    obj->is_optional = 1;
}

struct obj* obj_new(int type, const char* name, size_t length);
struct obj* obj_obj_new(const char* name, struct obj* children);
void obj_free(struct obj* obj);
const char* obj_strtype(const struct obj* obj);
const char* obj_strctype(const struct obj* obj);
void obj_dump(const struct obj* obj);


#endif /* OBJ_H_INCLUDED_ */

