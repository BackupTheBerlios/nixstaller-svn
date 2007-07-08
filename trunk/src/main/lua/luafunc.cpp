/*
    Copyright (C) 2007 Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.
    
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version. 
    
    This program is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE. See the GNU General Public License for more details. 
    
    You should have received a copy of the GNU General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
    St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "main/main.h"
#include "luafunc.h"
#include "luaclass.h"

namespace {

int DoFunctionCall(lua_State *L)
{
    lua_CFunction f = lua_tocfunction(L, lua_upvalueindex(2));
    
    try
    {
        int ret = f(L);
        return ret;
    }
    catch(Exceptions::CExUser &e)
    {
        // HACK: LoadFile/DoCall uses this to identify user exception.
        lua_pushstring(L, "CExUser");
        lua_error(L);
    }
    catch(Exceptions::CException &e)
    {
        // Lua doesn't handle exceptions in a nice way (catches all), so convert to lua error
        luaL_error(L, "Cought exception in lua function: %s", e.what());
    }

    return 0; // Never reached
}

}

namespace NLua {

// -------------------------------------
// Lua Function Wrapper Class
// -------------------------------------

CLuaFunc::CLuaFunc(const char *func, const char *tab) : m_bOK(true), m_iFuncRef(LUA_NOREF), m_iRetStartIndex(1),
                                                        m_ArgLuaTable(LUA_REGISTRYINDEX)
{
    GetGlobal(func, tab);
    if (lua_isnil(LuaState, -1))
    {
        lua_pop(LuaState, 1);
        m_bOK = false;
    }
    else
        m_iFuncRef = luaL_ref(LuaState, LUA_REGISTRYINDEX);
}

CLuaFunc::CLuaFunc(const char *func, const char *type, void *prvdata) : m_bOK(true), m_iFuncRef(LUA_NOREF),
                                                                        m_iRetStartIndex(1),
                                                                        m_ArgLuaTable(LUA_REGISTRYINDEX)
{
    PushClass(type, prvdata);

    if (!lua_isnil(LuaState, -1))
    {
        lua_getfield(LuaState, -1, func);
        lua_remove(LuaState, -2); // Remove class-table
    }
    
    if (lua_isnil(LuaState, -1))
    {
        lua_pop(LuaState, 1);
        m_bOK = false;
    }
    else
        m_iFuncRef = luaL_ref(LuaState, LUA_REGISTRYINDEX);
}

CLuaFunc::CLuaFunc(int ref, int tab) : m_bOK(true), m_iFuncRef(LUA_NOREF), m_iRetStartIndex(1),
                                       m_ArgLuaTable(LUA_REGISTRYINDEX)
{
    lua_rawgeti(LuaState, tab, ref);
    
    if (!lua_isfunction(LuaState, -1))
    {
        lua_pop(LuaState, 1);
        m_bOK = false;
    }
    else
        m_iFuncRef = luaL_ref(LuaState, LUA_REGISTRYINDEX);
}

CLuaFunc::~CLuaFunc()
{
    luaL_unref(LuaState, LUA_REGISTRYINDEX, m_iFuncRef);
}

void CLuaFunc::CheckSelf()
{
    if (!m_bOK)
        throw Exceptions::CExLua("Tried to use invalid LuaFunc");
}

int CLuaFunc::operator ()(int ret)
{
    CheckSelf();
    
    int oldtop = lua_gettop(LuaState);
    
    lua_rawgeti(LuaState, LUA_REGISTRYINDEX, m_iFuncRef);
    
    int size = m_ArgLuaTable.Size();
    for (int i=1; i<=size; i++)
        m_ArgLuaTable[i].GetTable();
    
    if (lua_pcall(LuaState, size, ret, 0) != 0)
    {
        const char *errmsg = lua_tostring(LuaState, -1);
        if (!errmsg)
            errmsg = "Unknown error!";
        else if (!strcmp(errmsg, "CExUser"))
            throw Exceptions::CExUser();
            
        throw Exceptions::CExLua(CreateText("Error running function: %s", errmsg));
    }
    
    const int top = lua_gettop(LuaState);
//     const int count = top - (oldtop - (m_ArgLuaTable.Size() + 1));
    const int count = top - oldtop;
    int retstart = top - count;

    m_iRetStartIndex = 1;
    m_ArgLuaTable.New(LUA_REGISTRYINDEX);
    
    while (retstart < top)
    {
        lua_pushvalue(LuaState, retstart);
        m_ArgLuaTable[retstart-count].SetTable();
        retstart++;
    }
    
    lua_pop(LuaState, count);
    return count;
}


// -------------------------------------
// Utility Functions
// -------------------------------------

void RegisterFunction(lua_CFunction f, const char *name, const char *tab, void *data)
{
    // data is NULL by default, but because we want the real function at a fixed closure position (2) we
    // push the userdata always.
    if (!tab)
    {
        lua_pushlightuserdata(LuaState, data);
        lua_pushcfunction(LuaState, f);
        lua_pushcclosure(LuaState, DoFunctionCall, 2);
        lua_setglobal(LuaState, name);
    }
    else
    {
        lua_getglobal(LuaState, tab);
        
        if (lua_isnil(LuaState, -1))
        {
            lua_pop(LuaState, 1);
            lua_newtable(LuaState);
        }
        
        lua_pushlightuserdata(LuaState, data);
        lua_pushcfunction(LuaState, f);
        lua_pushcclosure(LuaState, DoFunctionCall, 2);

        lua_setfield(LuaState, -2, name);
        lua_setglobal(LuaState, tab);
    }
}

void RegisterClassFunction(lua_CFunction f, const char *name, const char *type, void *data)
{
    GetClassMT(type);
    int mt = lua_gettop(LuaState);
    
    lua_pushstring(LuaState, name);
    
    lua_pushlightuserdata(LuaState, data);
    lua_pushcfunction(LuaState, f);
    lua_pushcclosure(LuaState, DoFunctionCall, 2);
    
    lua_settable(LuaState, mt);
    lua_pop(LuaState, mt);
}


}
