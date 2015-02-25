#ifndef LUA_OBJ_H_INCLUDED_
#define LUA_OBJ_H_INCLUDED_

struct lua_State;

int lua_obj_new(struct lua_State* L, const struct obj* obj);

#endif /* LUA_OBJ_H_INCLUDED_ */

