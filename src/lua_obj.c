#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "obj.h"

static struct obj* get_self(lua_State* L)
{
    /* { table } */
    lua_rawgeti(L, -1, 0);
    /* { table, userdata } */
    struct obj* self = (struct obj*)lua_touserdata(L, -1);
    assert(self);
    lua_pop(L, 1);
    /* { table } */
    return self;
}

static int get_new_obj(lua_State* L, struct obj* obj)
{
    lua_newtable(L);
    /* { table, table } */
    lua_pushlightuserdata(L, obj);
    /* { table, table, userdata } */
    lua_rawseti(L, -2, 0);
    /* { table, table } */

    lua_getmetatable(L, -2);
    /* { table, table, metatable } */
    lua_setmetatable(L, -2);
    /* { table, table } */

    return 1;
}

static int l_obj_next(lua_State* L)
{
    struct obj* self = get_self(L);
    struct obj* next = self->next;
    return next ? get_new_obj(L, next) : 0;
}

static int l_obj_type(lua_State* L)
{
    struct obj* self = get_self(L);
    lua_pushstring(L, obj_strtype(self));
    return 1;
}

static int l_obj_ctype(lua_State* L)
{
    struct obj* self = get_self(L);
    lua_pushstring(L, obj_strctype(self));
    return 1;
}

static int l_obj_name(lua_State* L)
{
    struct obj* self = get_self(L);
    lua_pushstring(L, self->name);
    return 1;
}

static int l_obj_length(lua_State* L)
{
    struct obj* self = get_self(L);
    lua_pushinteger(L, self->length);
    return 1;
}

static int l_obj_children(lua_State* L)
{
    struct obj* self = get_self(L);
    struct obj* children = self->children;
    return children ? get_new_obj(L, children) : 0;
}

static int l_obj_is_optional(lua_State* L)
{
    struct obj* self = get_self(L);
    lua_pushboolean(L, self->is_optional);
    return 1;
}

static int l_call_index(lua_State* L)
{
    /* { table, string } */
    const char* index = lua_tostring(L, -1);
    if(!index)
        return 0;

    lua_pop(L, 1);
    /* { table } */

    if(0 == strcmp(index, "next"))        return l_obj_next(L);
    if(0 == strcmp(index, "type"))        return l_obj_type(L);
    if(0 == strcmp(index, "name"))        return l_obj_name(L);
    if(0 == strcmp(index, "ctype"))       return l_obj_ctype(L);
    if(0 == strcmp(index, "length"))      return l_obj_length(L);
    if(0 == strcmp(index, "children"))    return l_obj_children(L);
    if(0 == strcmp(index, "is_optional")) return l_obj_is_optional(L);

    return 0;
}

int lua_obj_new(lua_State* L, const struct obj* obj)
{
    lua_newtable(L);
    /* { table } */
    lua_pushlightuserdata(L, (void*)obj);
    /* { table, userdata } */
    lua_rawseti(L, -2, 0);
    /* { table } */

    lua_newtable(L);
    /* { table, metatable } */
    lua_pushcfunction(L, l_call_index);
    /* { table, metatable, function } */
    lua_setfield(L, -2, "__index");
    /* { table, metatable } */

    lua_setmetatable(L, -2);
    /* { table } */

   return 1;
}

