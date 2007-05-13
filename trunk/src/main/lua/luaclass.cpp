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

void CreateClass(void *prvdata, const char *type)
{
    lua_newtable(LuaState);
    int table = lua_gettop(LuaState);

    GetClassMT(type);
    int mt = lua_gettop(LuaState);

    // mt[prvdata] = table
    lua_pushlightuserdata(LuaState, reinterpret_cast<void *>(prvdata));
    lua_pushvalue(LuaState, table);
    lua_settable(LuaState, mt);

    // mt[table] = prvdata
    lua_pushvalue(LuaState, table);
    lua_pushlightuserdata(LuaState, reinterpret_cast<void *>(prvdata));
    lua_settable(LuaState, mt);

    lua_setmetatable(LuaState, table);
}

void SetClassGC(const char *name, lua_CFunction gc, void *gcdata)
{
    GetClassMT(name);
    
    lua_pushstring(LuaState, "__gc");
    
    if (gcdata)
    {
        lua_pushlightuserdata(LuaState, gcdata);
        lua_pushcclosure(LuaState, gc, 1);
    }
    else
        lua_pushcfunction(LuaState, gc);
    
    lua_settable(LuaState, -3);
    lua_pop(LuaState, 1);
}


}
