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
#include "luaprogressbar.h"

// -------------------------------------
// Base Lua Progress Bar Class
// -------------------------------------

void CBaseLuaProgressBar::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaProgressBar::LuaSet, "set", "progressbar");
}

int CBaseLuaProgressBar::LuaSet(lua_State *L)
{
    CBaseLuaProgressBar *bar = NLua::CheckClassData<CBaseLuaProgressBar>("progressbar", 1);
    int n = luaL_checkint(L, 2);
    
    if ((n < 0) || (n > 100))
        luaL_error(L, "Wrong value specified, should be between 0-100");
    
    bar->SetProgress(n);
    return 0;
}
