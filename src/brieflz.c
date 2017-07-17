#include <lua.h>
#include <lauxlib.h>
#include <stddef.h>
#include <stdlib.h>


static const struct luaL_Reg brieflz_funcs[] = {
    { NULL    , NULL           },
};

int luaopen_brieflz(lua_State *L) {
    luaL_newlib(L, brieflz_funcs);

    return 1;  // number of result values
}
