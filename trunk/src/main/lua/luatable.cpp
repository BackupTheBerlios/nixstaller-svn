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

#include "luatable.h"
#include "luaclass.h"

namespace NLua {

// -------------------------------------
// Lua Table Wrapper Class
// -------------------------------------

CLuaTable::CLuaTable(const std::string &var, const std::string &tab) : m_bOK(true)
{
    if (!tab.empty())
    {
        GetTable(tab, LUA_GLOBALSINDEX);
        GetTable(var, lua_gettop(LuaState));
        lua_remove(LuaState, -2);
    }
    else
        GetTable(var, LUA_GLOBALSINDEX);
    
    m_iTabIndex = lua_gettop(LuaState);
}

CLuaTable::CLuaTable(const std::string &var, const std::string &type, void *prvdata) : m_bOK(true)
{
    PushClass(type, prvdata);
    int tab = lua_gettop(LuaState);
    
    if (lua_isnil(LuaState, tab))
        m_bOK = false;
    else
    {
        lua_getfield(LuaState, tab, var.c_str());
        
        if (lua_isnil(LuaState, -1))
        {
            lua_pop(LuaState, 1);
            
            lua_newtable(LuaState);
            
            lua_pushvalue(LuaState, -1);
            lua_setfield(LuaState, tab, var.c_str());
        }
    }
    
    lua_remove(LuaState, tab);
}

void CLuaTable::GetTable(const std::string &tab, int index)
{
    lua_getfield(LuaState, index, tab.c_str());
    
    if (lua_isnil(LuaState, -1))
    {
        lua_pop(LuaState, 1);
        
        lua_newtable(LuaState);
        lua_pushvalue(LuaState, -1);
        lua_setglobal(LuaState, tab.c_str());
    }
}

// -------------------------------------
// Lua Table Return Wrapper Class
// -------------------------------------

void CLuaTable::CReturn::PushIndex()
{
    if (m_iIndexType == LUA_TSTRING)
        lua_pushstring(LuaState, m_Index.c_str());
    else
        lua_pushinteger(LuaState, m_iIndex);
}

void CLuaTable::CReturn::operator <<(const std::string &val)
{
    PushIndex();
    lua_pushstring(LuaState, val.c_str());
    lua_settable(LuaState, m_iTabIndex);
}

void CLuaTable::CReturn::operator <<(int val)
{
    PushIndex();
    lua_pushinteger(LuaState, val);
    lua_settable(LuaState, m_iTabIndex);
}

void CLuaTable::CReturn::operator >>(std::string &val)
{
    PushIndex();
    lua_gettable(LuaState, m_iTabIndex);
    
    val = luaL_checkstring(LuaState, -1);
    
    lua_pop(LuaState, 1);
}

void CLuaTable::CReturn::operator >>(int &val)
{
    PushIndex();
    lua_gettable(LuaState, m_iTabIndex);
    
    val = luaL_checkint(LuaState, -1);
    
    lua_pop(LuaState, 1);
}


}
