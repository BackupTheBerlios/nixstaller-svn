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
#include "luatable.h"
#include "luaclass.h"

namespace NLua {

// -------------------------------------
// Lua Table Wrapper Class
// -------------------------------------

CLuaTable::CLuaTable(const char *var, const char *tab) : m_bOK(true), m_iTabRef(LUA_NOREF)
{
    Open(var, tab);
}

CLuaTable::CLuaTable(const char *var, const char *type, void *prvdata) : m_bOK(true), m_iTabRef(LUA_NOREF)
{
    Open(var, type, prvdata);
}

CLuaTable::CLuaTable(int tab) : m_bOK(true), m_iTabRef(LUA_NOREF)
{
    New(tab);
}

void CLuaTable::GetTable(const std::string &tab, int index)
{
    lua_getfield(LuaState, index, tab.c_str());
    
    if (lua_isnil(LuaState, -1))
    {
        lua_pop(LuaState, 1);
        
        lua_newtable(LuaState);
        lua_pushvalue(LuaState, -1);
        lua_setfield(LuaState, index, tab.c_str());
    }
}

void CLuaTable::GetTable(int ref, int index)
{
    lua_rawgeti(LuaState, index, ref);
    
    if (lua_isnil(LuaState, -1))
    {
        lua_pop(LuaState, 1);
        
        lua_newtable(LuaState);
        lua_pushvalue(LuaState, -1);
        lua_rawseti(LuaState, index, ref);
    }
}

void CLuaTable::CheckSelf()
{
    if (!m_bOK)
        throw Exceptions::CExLua("Tried to use invalid or closed table");
}

void CLuaTable::New(int tab)
{
    Close();
    m_bOK = true;
    lua_newtable(LuaState);
    m_iTabRef = luaL_ref(LuaState, tab);
}

void CLuaTable::Open(const char *var, const char *tab)
{
    Close();
    m_bOK = true;
    
    if (tab)
    {
        GetTable(tab, LUA_GLOBALSINDEX);
        GetTable(var, lua_gettop(LuaState));
        lua_remove(LuaState, -2);
    }
    else
        GetTable(var, LUA_GLOBALSINDEX);
    
    m_iTabRef = luaL_ref(LuaState, LUA_REGISTRYINDEX);
}

void CLuaTable::Open(const char *var, const char *type, void *prvdata)
{
    Close();

    PushClass(type, prvdata);
    int tab = lua_gettop(LuaState);
    
    if (lua_isnil(LuaState, tab))
        m_bOK = false;
    else
    {
        m_bOK = true;

        lua_getfield(LuaState, tab, var);
        
        if (lua_isnil(LuaState, -1))
        {
            lua_pop(LuaState, 1);
            
            lua_newtable(LuaState);
            
            lua_pushvalue(LuaState, -1);
            lua_setfield(LuaState, tab, var);
        }
        
        m_iTabRef = luaL_ref(LuaState, LUA_REGISTRYINDEX);
    }
    
    lua_remove(LuaState, tab);
}

void CLuaTable::Close()
{
    luaL_unref(LuaState, LUA_REGISTRYINDEX, m_iTabRef);
    m_iTabRef = LUA_NOREF;
    m_bOK = false;
}

int CLuaTable::Size()
{
    if (!m_bOK)
        return 0;
    
    lua_rawgeti(LuaState, LUA_REGISTRYINDEX, m_iTabRef);
    int ret = lua_objlen(LuaState, -1);
    lua_pop(LuaState, 1);
    return ret;
}

// -------------------------------------
// Lua Table Return Wrapper Class
// -------------------------------------

CLuaTable::CReturn::CReturn(const std::string &index, int tab) : m_iTabRef(tab)
{
    lua_pushstring(LuaState, index.c_str());
    m_iIndexRef = luaL_ref(LuaState, LUA_REGISTRYINDEX);
}

CLuaTable::CReturn::CReturn(int index, int tab) : m_iTabRef(tab)
{
    lua_pushinteger(LuaState, index);
    m_iIndexRef = luaL_ref(LuaState, LUA_REGISTRYINDEX);
}

CLuaTable::CReturn::~CReturn()
{
    luaL_unref(LuaState, LUA_REGISTRYINDEX, m_iIndexRef);
}

void CLuaTable::CReturn::SetTable()
{
    lua_rawgeti(LuaState, LUA_REGISTRYINDEX, m_iIndexRef);
    lua_insert(LuaState, -2); // swap value <--> key
    
    lua_rawgeti(LuaState, LUA_REGISTRYINDEX, m_iTabRef);
    lua_insert(LuaState, -3); // Move it before key and value

    lua_settable(LuaState, -3);
    lua_pop(LuaState, 1); // Remove table
}

void CLuaTable::CReturn::GetTable()
{
    lua_rawgeti(LuaState, LUA_REGISTRYINDEX, m_iTabRef);
    int tab = lua_gettop(LuaState);
    
    lua_rawgeti(LuaState, LUA_REGISTRYINDEX, m_iIndexRef);
    
    lua_gettable(LuaState, tab);
    lua_remove(LuaState, tab);
}

void CLuaTable::CReturn::operator <<(const std::string &val)
{
    lua_pushstring(LuaState, val.c_str());
    SetTable();
}

void CLuaTable::CReturn::operator <<(const char *val)
{
    lua_pushstring(LuaState, val);
    SetTable();
}

void CLuaTable::CReturn::operator <<(int val)
{
    lua_pushinteger(LuaState, val);
    SetTable();
}

void CLuaTable::CReturn::operator <<(bool val)
{
    lua_pushboolean(LuaState, val);
    SetTable();
}

void CLuaTable::CReturn::operator >>(std::string &val)
{
    GetTable();
    val = luaL_checkstring(LuaState, -1);
    lua_pop(LuaState, 1);
}

void CLuaTable::CReturn::operator >>(const char *&val)
{
    GetTable();
    val = luaL_checkstring(LuaState, -1);
    lua_pop(LuaState, 1);
}

void CLuaTable::CReturn::operator >>(int &val)
{
    GetTable();
    val = luaL_checkint(LuaState, -1);
    lua_pop(LuaState, 1);
}

void CLuaTable::CReturn::operator >>(bool &val)
{
    GetTable();
    luaL_checktype(LuaState, -1, LUA_TBOOLEAN);
    val = lua_toboolean(LuaState, -1);
    lua_pop(LuaState, 1);
}


}
