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
    ssize_t length;
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

