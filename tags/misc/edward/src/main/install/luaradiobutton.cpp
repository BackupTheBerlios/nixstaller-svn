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

#include "main/main.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "luaradiobutton.h"

// -------------------------------------
// Base Lua Radio Button Class
// -------------------------------------

void CBaseLuaRadioButton::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaRadioButton::LuaGet, "get", "radiobutton");
    NLua::RegisterClassFunction(CBaseLuaRadioButton::LuaSet, "set", "radiobutton");
}

int CBaseLuaRadioButton::LuaGet(lua_State *L)
{
    CBaseLuaRadioButton *box = CheckLuaWidgetClass<CBaseLuaRadioButton>("radiobutton", 1);
    lua_pushstring(L, box->EnabledButton());
    return 1;
}

int CBaseLuaRadioButton::LuaSet(lua_State *L)
{
    CBaseLuaRadioButton *box = CheckLuaWidgetClass<CBaseLuaRadioButton>("radiobutton", 1);
    int vartype = lua_type(L, 2);
    TSTLVecSize n = 0;
    
    if (vartype == LUA_TNUMBER)
        n = SafeConvert<TSTLVecSize>(lua_tointeger(L, 2)) - 1;
    else if (vartype == LUA_TSTRING)
        n = std::distance(box->m_Options.begin(),
                          std::find(box->m_Options.begin(), box->m_Options.end(), lua_tostring(L, 2)));
    else
        luaL_typerror(L, 2, "Number or String");

    luaL_argcheck(L, ((n >= 0) && (n < box->m_Options.size())), 2, "Tried to set non existing radiobutton entry");
    
    box->Enable(n);
    return 0;
}
