/***
 * Lua binding for BriefLZ library.
 *
 * @module brieflz
 */
#include <lua.h>
#include <lauxlib.h>
#include <stddef.h>
#include <stdlib.h>

#include "brieflz.h"


#define LIBNAME "brieflz"
#define VERSION "0.1.0"

// Copied from Lua 5.3 lauxlib.h for compatibility with Lua <5.3.
#if !defined(lua_writestringerror)
    #define lua_writestringerror(s,p) \
            (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

// Copied from lua-compat-5.2 for compatibility with Lua 5.1.
#if !defined(luaL_newlib)
    #define luaL_newlib(L, l) \
        (lua_newtable((L)),luaL_setfuncs((L), (l), 0))
#endif

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM == 501  // Lua 5.1

    // Copied from lua-compat-5.2 for compatibility with Lua 5.1.
    void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
        luaL_checkstack(L, nup+1, "too many upvalues");

        for (; l->name != NULL; l++) {
            int i;
            lua_pushstring(L, l->name);
            for (i = 0; i < nup; i++)  {
                lua_pushvalue(L, -(nup + 1));
            }
            lua_pushcclosure(L, l->func, nup);
            lua_settable(L, -(nup + 3));
        }
        lua_pop(L, nup);
    }
#endif  // Lua 5.1


/***
 * Compresses the given `data` using BriefLZ algorithm.
 *
 * This is a **low-level function**, it does not add any headers to the packed
 * data! You must know the exact size of (uncompressed) data to be able to
 * depack it.
 *
 * @function pack
 * @tparam string data The data to compress.
 * @treturn string Compressed data.
 * @treturn int Size of the (uncompressed) `data`.
 * @treturn int Size of the compressed data.
 *
 * @see depack
 */
static int brieflz_pack (lua_State *L) {
    size_t src_size;
    const char *src = luaL_checklstring(L, 1, &src_size);  // arg #1: data

    void *ud;
    const lua_Alloc alloc = lua_getallocf(L, &ud);

    const size_t dest_size = blz_max_packed_size(src_size);
    const size_t workmem_size = blz_workmem_size(src_size);

    void *dest = alloc(ud, NULL, 0, dest_size);  // malloc
    void *workmem = alloc(ud, NULL, 0, workmem_size);  // malloc
    if (dest == NULL || workmem == NULL) {
        return luaL_error(L, "not enough memory to pack data");
    }

    const unsigned long packed_size = blz_pack(src, dest, src_size, workmem);

    // Push result values to the stack.
    lua_pushlstring(L, (const char*) dest, (size_t) packed_size);
    lua_pushinteger(L, (lua_Integer) src_size);
    lua_pushinteger(L, (lua_Integer) packed_size);

    (void) alloc(ud, workmem, workmem_size, 0);  // free
    (void) alloc(ud, dest, dest_size, 0);  // free

    return 3;  // number of result values
}

/***
 * Decompresses the given `data` using BriefLZ algorithm.
 *
 * This is very **low-level function**. It does not expect any headers in the
 * packed data, so you must know the exact size of uncompressed data! If you
 * specify incorrect `depacked_size`, it raises an error. However, it's
 * memory-safe, i.e. it will not segfault in this case.
 *
 * @function depack
 * @tparam string data The compressed data.
 * @tparam int depacked_size Size of the decompressed data (must exactly match!).
 * @treturn string Uncompressed data.
 * @raise if the `data` or `depacked_size` is invalid.
 *
 * @see pack
 */
static int brieflz_depack (lua_State *L) {
    size_t src_size;

    const char *src = luaL_checklstring(L, 1, &src_size);   // arg #1: data
    const lua_Integer dest_size = luaL_checkinteger(L, 2);  // arg #2: depacked_size

    // If the given string is empty or dest_size is 0, then just return it.
    if (src_size == 0 || dest_size == 0) {
        lua_pushlstring(L, src, 0);
        return 1;  // number of result values
    }
    if (dest_size < 0) {
        return luaL_argerror(L, 2, "must be positive integer");
    }

    void *ud;
    const lua_Alloc alloc = lua_getallocf(L, &ud);

    void *dest = alloc(ud, NULL, 0, dest_size);  // malloc
    if (dest == NULL) {
        return luaL_error(L, "not enough memory to depack data");
    }

    if (blz_depack_safe(src, src_size, dest, dest_size) != dest_size) {
        (void) alloc(ud, dest, dest_size, 0);  // free
        return luaL_error(L, "failed to depack data");
    }

    lua_pushlstring(L, (const char*) dest, (size_t) dest_size);

    (void) alloc(ud, dest, dest_size, 0);  // free

    return 1;  // number of result values
}


static const struct luaL_Reg brieflz_funcs[] = {
    { "pack"  , brieflz_pack   },
    { "depack", brieflz_depack },
    { NULL    , NULL           },
};

int luaopen_brieflz(lua_State *L) {
    luaL_newlib(L, brieflz_funcs);

    /// @field _NAME Name of this module.
    lua_pushliteral(L, LIBNAME);
    lua_setfield(L, -2, "_NAME");

    /// @field _VERSION Version of this module.
    lua_pushliteral(L, VERSION);
    lua_setfield(L, -2, "_VERSION");

    /// @field _BLZ_VERSION Version of the BriefLZ C library.
    lua_pushliteral(L, BLZ_VER_STRING);
    lua_setfield(L, -2, "_BLZ_VERSION");

    return 1;  // number of result values
}
