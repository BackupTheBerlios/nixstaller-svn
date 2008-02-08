/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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

#include "luaclass.h"

namespace NLua {

void GetClassMT(const std::string &type)
{
    bool init = (luaL_newmetatable(LuaState, type.c_str()) != 0);

    if (init)
    {
        int mt = lua_gettop(LuaState);

        lua_pushstring(LuaState, "__index");
        lua_pushvalue(LuaState, mt);
        lua_settable(LuaState, mt); // __index = metatable
        
        lua_pushstring(LuaState, "__metatable");
//         lua_pushvalue(LuaState, mt);
        lua_pushstring(LuaState, "private");
        lua_settable(LuaState, mt); // hide metatable
    }
}

void PushClass(const std::string &type, void *prvdata)
{
    GetClassMT(type);

    // get mt[prvdata]
    lua_pushlightuserdata(LuaState, prvdata);
    lua_gettable(LuaState, -2);
    
    lua_remove(LuaState, -2); // Remove metatable
}

void PushClassData(const char *type, int index)
{
    index = AbsoluteIndex(index);
    
    bool hastab = lua_getmetatable(LuaState, index) != 0;
    int mt = lua_gettop(LuaState);

    if (hastab)
    {
        lua_getfield(LuaState, LUA_REGISTRYINDEX, type);
        int field = lua_gettop(LuaState);
        
        if (lua_rawequal(LuaState, -1, -2))
        {
            // get mt[table]
            lua_pushvalue(LuaState, index);
            lua_gettable(LuaState, mt);
        }
        lua_remove(LuaState, field);
    }
    
    lua_remove(LuaState, mt);
}

void CreateClass(void *prvdata, const char *type)
{
    lua_newtable(LuaState);
    int table = lua_gettop(LuaState);

    GetClassMT(type);
    int mt = lua_gettop(LuaState);

    // mt[prvdata] = table
    lua_pushlightuserdata(LuaState, static_cast<void *>(prvdata));
    lua_pushvalue(LuaState, table);
    lua_settable(LuaState, mt);

    // mt[table] = prvdata
    lua_pushvalue(LuaState, table);
    lua_pushlightuserdata(LuaState, static_cast<void *>(prvdata));
    lua_settable(LuaState, mt);

    lua_setmetatable(LuaState, table);
}

void DeleteClass(void *prvdata, const char *type)
{
    GetClassMT(type);
    int mt = lua_gettop(LuaState);
    PushClass(type, prvdata);
    int table = lua_gettop(LuaState);

    // Clear mt[prvdata]
    lua_pushlightuserdata(LuaState, static_cast<void *>(prvdata));
    lua_pushnil(LuaState);
    lua_settable(LuaState, mt);

    // Clear mt[table]
    lua_pushvalue(LuaState, table);
    lua_pushnil(LuaState);
    lua_settable(LuaState, mt);

    lua_remove(LuaState, table);
    lua_remove(LuaState, mt);
}


}
