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

#ifndef LUA_CLASS_H
#define LUA_CLASS_H

#include "lua.h"

namespace NLua {

void GetClassMT(const std::string &type);
void PushClass(const std::string &type, void *prvdata);
void PushClassData(const char *type, int index);
void CreateClass(void *prvdata, const char *type);

template <typename C> C *ToClassData(const char *type, int index)
{
    PushClassData(type, index);
    
    C *ret = NULL;
    
    if (!lua_isnil(LuaState, -1))
        ret = reinterpret_cast<C *>(lua_touserdata(LuaState, -1));
    
    lua_pop(LuaState, 1);
    
    return ret;
}

template <typename C> C *CheckClassData(const char *type, int index)
{
    luaL_checktype(LuaState, index, LUA_TTABLE);
    C *ret = ToClassData<C>(type, index);
    
    if (!ret)
        luaL_typerror(LuaState, index, type);
    
    return ret;
}

void SetClassGC(const char *name, lua_CFunction gc, void *gcdata=NULL);

}

#endif
