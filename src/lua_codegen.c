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

