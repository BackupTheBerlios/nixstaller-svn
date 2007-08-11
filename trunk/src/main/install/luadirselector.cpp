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

#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "luadirselector.h"

// -------------------------------------
// Base Lua Directory Selector Class
// -------------------------------------

void CBaseLuaDirSelector::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaDirSelector::LuaGet, "get", "dirselector");
    NLua::RegisterClassFunction(CBaseLuaDirSelector::LuaSet, "set", "dirselector");
}

int CBaseLuaDirSelector::LuaGet(lua_State *L)
{
    CBaseLuaDirSelector *sel = NLua::CheckClassData<CBaseLuaDirSelector>("dirselector", 1);
    lua_pushstring(L, sel->GetDir());
    return 1;
}

int CBaseLuaDirSelector::LuaSet(lua_State *L)
{
    CBaseLuaDirSelector *sel = NLua::CheckClassData<CBaseLuaDirSelector>("dirselector", 1);
    const char *dir = luaL_checkstring(L, 2);
    sel->SetDir(dir);
    return 0;
}
