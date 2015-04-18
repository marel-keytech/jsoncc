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

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "obj.h"
#include "lua_obj.h"

int lua_codegen(const char* template, const char* name, const struct obj* obj)
{
    int r = 0;
    lua_State* L = luaL_newstate();
    if(!L)
        return -1;

    luaL_openlibs(L);

    lua_pushstring(L, name);
    lua_setglobal(L, "JSON_NAME");

    lua_obj_new(L, obj);
    lua_setglobal(L, "JSON_ROOT");

    if(luaL_dofile(L, template) != 0)
    {
        fprintf(stderr, "Running template failed: %s\n", lua_tostring(L, -1));
        r = -1;
    }

    lua_close(L);

    return r;
}

