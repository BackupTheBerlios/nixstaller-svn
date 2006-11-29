/*
    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.

    Nixstaller is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Nixstaller is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nixstaller; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
    conditions of the GNU General Public License cover the whole combination.

    In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
    software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
    DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
    such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
    you include the source code of that other code when and as the GNU GPL requires distribution of source code.

    Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
    versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
    version without this exception; this exception also makes it possible to release a modified version which carries forward
    this exception.
*/

#include "main.h"

// Table of libs that should be loaded
static const luaL_Reg libtable[] = {
    { "", luaopen_base },
    { LUA_TABLIBNAME, luaopen_table },
    { LUA_IOLIBNAME, luaopen_io },
    { LUA_OSLIBNAME, luaopen_os },
    { LUA_STRLIBNAME, luaopen_string },
    { LUA_MATHLIBNAME, luaopen_math },
 // { LUA_LOADLIBNAME, luaopen_package }, // Perhaps later
  { LUA_DBLIBNAME, luaopen_debug }, // No debug
    { NULL, NULL }
};
    
// -------------------------------------
// Lua Class
// -------------------------------------

#ifndef RELEASE
// Based from a example in the book "Programming in Lua"
void CLuaVM::StackDump(const char *msg)
{
    if (msg)
        debugline(msg);
    
    int i;
    int top = lua_gettop(m_pLuaState);
    for (i = 1; i <= top; i++)
    {
        /* repeat for each level */
        int t = lua_type(m_pLuaState, i);
        switch (t)
        {
            case LUA_TSTRING:  /* strings */
                debugline("`%s'", lua_tostring(m_pLuaState, i));
                break;
            case LUA_TBOOLEAN:  /* booleans */
                debugline(lua_toboolean(m_pLuaState, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:  /* numbers */
                debugline("%g", lua_tonumber(m_pLuaState, i));
                break;
            default:  /* other values */
                debugline("%s", lua_typename(m_pLuaState, t));
                break;
        }
        debugline("  ");  /* put a separator */
    }
    debugline("\n");  /* end the listing */
}
#endif

void CLuaVM::GetGlobal(const char *var, const char *tab)
{
    if (tab)
    {
        lua_getglobal(m_pLuaState, tab);
        if (!lua_isnil(m_pLuaState, -1))
        {
            lua_getfield(m_pLuaState, -1, var);
            lua_remove(m_pLuaState, -2); // Remove table from stack(we don't need it anymore)
        }
    }
    else
        lua_getglobal(m_pLuaState, var);
}

int CLuaVM::DoFunctionCall(lua_State *L)
{
    lua_CFunction f = lua_tocfunction(L, lua_upvalueindex(2));
    
    try
    {
        int ret = f(L);
        return ret;
    }
    catch(Exceptions::CException &e)
    {
        // Lua doesn't handle exceptions in a nice way (catches all), so convert to lua error
        luaL_error(L, "Cought exception in lua function: %s", e.what());
    }
    return 0; // Never reached
}

void CLuaVM::Init()
{
    // Initialize lua
    m_pLuaState = lua_open();
    
    if (!m_pLuaState)
        throw Exceptions::CExLua("Could not open lua VM");
        
    const luaL_Reg *lib = libtable;
    for (; lib->func; lib++)
    {
        lua_pushcfunction(m_pLuaState, lib->func);
        lua_pushstring(m_pLuaState, lib->name);
        lua_call(m_pLuaState, 1, 0);
        lua_settop(m_pLuaState, 0);  // Clear stack
    }
}

void CLuaVM::LoadFile(const char *name)
{
    if (luaL_dofile(m_pLuaState, name))
    {
        const char *errmsg = lua_tostring(m_pLuaState, -1);
        if (!errmsg)
            errmsg = "Unknown error!";
        throw Exceptions::CExLua(CreateText("While parsing %s: %s\n", name, errmsg));
    }
}

void CLuaVM::RegisterFunction(lua_CFunction f, const char *name, const char *tab, void *data)
{
    // data is NULL by default, but because we want the real function at a fixed closure position (2) we push the userdata always.
    if (!tab)
    {
        lua_pushlightuserdata(m_pLuaState, data);
        lua_pushcfunction(m_pLuaState, f);
        lua_pushcclosure(m_pLuaState, DoFunctionCall, 2);
        lua_setglobal(m_pLuaState, name);
    }
    else
    {
        lua_getglobal(m_pLuaState, tab);
        
        if (lua_isnil(m_pLuaState, -1))
        {
            lua_pop(m_pLuaState, 1);
            lua_newtable(m_pLuaState);
        }
        
        lua_pushlightuserdata(m_pLuaState, data);
        lua_pushcfunction(m_pLuaState, f);
        lua_pushcclosure(m_pLuaState, DoFunctionCall, 2);

        lua_setfield(m_pLuaState, -2, name);
        lua_setglobal(m_pLuaState, tab);
    }
}

void CLuaVM::RegisterNumber(lua_Number n, const char *name, const char *tab)
{
    if (!tab)
    {
        lua_pushnumber(m_pLuaState, n);
        lua_setglobal(m_pLuaState, name);
    }
    else
    {
        lua_getglobal(m_pLuaState, tab);
        
        if (lua_isnil(m_pLuaState, -1))
        {
            lua_pop(m_pLuaState, 1);
            lua_newtable(m_pLuaState);
        }
        
        lua_pushnumber(m_pLuaState, n);
        lua_setfield(m_pLuaState, -2, name);
        
        lua_setglobal(m_pLuaState, tab);
    }
}

void CLuaVM::RegisterString(const char *s, const char *name, const char *tab)
{
    if (!tab)
    {
        lua_pushstring(m_pLuaState, s);
        lua_setglobal(m_pLuaState, name);
    }
    else
    {
        lua_getglobal(m_pLuaState, tab);
        
        if (lua_isnil(m_pLuaState, -1))
        {
            lua_pop(m_pLuaState, 1);
            lua_newtable(m_pLuaState);
        }
        
        lua_pushstring(m_pLuaState, s);
        lua_setfield(m_pLuaState, -2, name);
        
        lua_setglobal(m_pLuaState, tab);
    }
}

bool CLuaVM::InitCall(const char *func, const char *tab)
{
    GetGlobal(func, tab);
        
    if (lua_isnil(m_pLuaState, -1))
    {
        lua_pop(m_pLuaState, 1);
        return false;
    }

    return true;
}

void CLuaVM::PushArg(lua_Number n)
{
    lua_pushnumber(m_pLuaState, n);
    m_iPushedArgs++;
}

void CLuaVM::PushArg(const char *s)
{
    lua_pushstring(m_pLuaState, s);
    m_iPushedArgs++;
}

void CLuaVM::DoCall(void)
{
    if (lua_pcall(m_pLuaState, m_iPushedArgs, 0, 0) != 0)
        throw Exceptions::CExLua(CreateText("Error running function: %s", lua_tostring(m_pLuaState, -1)));
    
    m_iPushedArgs = 0;
}

bool CLuaVM::GetNumVar(lua_Number *out, const char *var, const char *tab)
{
    GetGlobal(var, tab);
    
    if (lua_isnil(m_pLuaState, -1))
    {
        lua_pop(m_pLuaState, 1);
        return false;
    }
    
    bool goodtype = lua_isnumber(m_pLuaState, -1);
    if (goodtype)
        *out = lua_tonumber(m_pLuaState, -1);
    
    lua_pop(m_pLuaState, 1);
    
    return (goodtype);
}

bool CLuaVM::GetNumVar(lua_Integer *out, const char *var, const char *tab)
{
    GetGlobal(var, tab);
    
    if (lua_isnil(m_pLuaState, -1))
    {
        lua_pop(m_pLuaState, 1);
        return false;
    }
    
    bool goodtype = lua_isnumber(m_pLuaState, -1);
    if (goodtype)
        *out = lua_tointeger(m_pLuaState, -1);
    
    lua_pop(m_pLuaState, 1);
    
    return (goodtype);
}

bool CLuaVM::GetStrVar(std::string *out, const char *var, const char *tab)
{
    GetGlobal(var, tab);
    
    if (lua_isnil(m_pLuaState, -1))
    {
        lua_pop(m_pLuaState, 1);
        return false;
    }
    
    bool goodtype = lua_isstring(m_pLuaState, -1);
    if (goodtype)
        *out = lua_tostring(m_pLuaState, -1);
    
    lua_pop(m_pLuaState, 1);
    
    return (goodtype);
}

const char *CLuaVM::GetStrVar(const char *var, const char *tab)
{
    const char *out = NULL;
    GetGlobal(var, tab);
    
    if (lua_isnil(m_pLuaState, -1))
    {
        lua_pop(m_pLuaState, 1);
        return NULL;
    }
    
    bool goodtype = lua_isstring(m_pLuaState, -1);
    if (goodtype)
        out = lua_tostring(m_pLuaState, -1);
    
    lua_pop(m_pLuaState, 1);
    
    return out;
}

unsigned CLuaVM::OpenArray(const char *var, const char *tab)
{
    GetGlobal(var, tab);
        
    if (lua_isnil(m_pLuaState, -1))
    {
        lua_pop(m_pLuaState, 1);
        return 0;
    }
    
    return luaL_getn(m_pLuaState, -1);
}

bool CLuaVM::GetArrayNum(unsigned &index, lua_Number *out)
{
    lua_rawgeti(m_pLuaState, -1, index);
    
    bool goodtype = lua_isnumber(m_pLuaState, -1);
    if (goodtype)
        *out = lua_tonumber(m_pLuaState, -1);
    
    lua_pop(m_pLuaState, 1);
    
    return (goodtype);
}

bool CLuaVM::GetArrayNum(unsigned &index, lua_Integer *out)
{
    lua_rawgeti(m_pLuaState, -1, index);
    
    bool goodtype = lua_isnumber(m_pLuaState, -1);
    if (goodtype)
        *out = lua_tointeger(m_pLuaState, -1);
    
    lua_pop(m_pLuaState, 1);
    
    return (goodtype);
}

bool CLuaVM::GetArrayStr(unsigned &index, std::string *out)
{
    lua_rawgeti(m_pLuaState, -1, index);
    
    bool goodtype = lua_isstring(m_pLuaState, -1);
    if (goodtype)
        *out = lua_tostring(m_pLuaState, -1);
    
    lua_pop(m_pLuaState, 1);
    
    return (goodtype);
}

void CLuaVM::CloseArray()
{
    if (lua_istable(m_pLuaState, -1))
        lua_pop(m_pLuaState, 1);
}

void *CLuaVM::GetClosure()
{
    return lua_touserdata(m_pLuaState, lua_upvalueindex(1));
}

void CLuaVM::InitClass(const char *name, lua_CFunction gc, void *gcdata)
{
    luaL_newmetatable(m_pLuaState, name);
    
    lua_pushstring(m_pLuaState, "__index");
    lua_pushvalue(m_pLuaState, -2);
    lua_settable(m_pLuaState, -3); // __index = metatable
    
    lua_pushstring(m_pLuaState, "__metatable");
    lua_pushvalue(m_pLuaState, -2);
    lua_settable(m_pLuaState, -3); // hide metatable
    
    if (gc)
    {
        lua_pushstring(m_pLuaState, "__gc");
        
        if (gcdata)
        {
            lua_pushlightuserdata(m_pLuaState, gcdata);
            lua_pushcclosure(m_pLuaState, gc, 1);
        }
        else
            lua_pushcfunction(m_pLuaState, gc);
        
        lua_settable(m_pLuaState, -3);
    }
    
    lua_pop(m_pLuaState, 1);
}

void CLuaVM::RegisterClassFunc(const char *type, lua_CFunction f, const char *name, void *data)
{
    luaL_getmetatable(m_pLuaState, type);
    
    lua_pushstring(m_pLuaState, name);
    
    if (data)
    {
        lua_pushlightuserdata(m_pLuaState, data);
        lua_pushcclosure(m_pLuaState, f, 1);
    }
    else
        lua_pushcfunction(m_pLuaState, f);
    
    lua_settable(m_pLuaState, -3);
}

void CLuaVM::SetArrayStr(const char *s, const char *tab, int index)
{
    lua_getglobal(m_pLuaState, tab);

    if (lua_isnil(m_pLuaState, -1))
    {
        lua_pop(m_pLuaState, 1);
        lua_newtable(m_pLuaState);
    }

    lua_pushstring(m_pLuaState, s);
    lua_rawseti(m_pLuaState, -2, index);
    lua_setglobal(m_pLuaState, tab);
}

void CLuaVM::LuaError(const char *msg, ...)
{
    static char fmt[1024];
    va_list v;
    
    va_start(v, msg);
    vsnprintf(fmt, sizeof(fmt), msg, v);
    va_end(v);
    
    luaL_error(m_pLuaState, fmt);
}
