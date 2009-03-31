/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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

#include <stdlib.h>
#include <string.h>

#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "luainput.h"

// -------------------------------------
// Base Lua Inputfield Class
// -------------------------------------

CBaseLuaInputField::CBaseLuaInputField(const char *l, const char *t) : m_iLabelWidth(0)
{
    if (l && *l)
        m_Label = l;
    
    if (!t || !t[0] || (strcmp(t, "number") && strcmp(t, "float") && strcmp(t, "string")))
        m_szType = "string";
    else
        m_szType = t;
}

void CBaseLuaInputField::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaInputField::LuaSet, "set", "inputfield");
    NLua::RegisterClassFunction(CBaseLuaInputField::LuaGet, "get", "inputfield");
    NLua::RegisterClassFunction(CBaseLuaInputField::LuaSetLabelWidth, "setlabelwidth", "inputfield");
}

int CBaseLuaInputField::LuaSet(lua_State *L)
{
    CBaseLuaInputField *field = CheckLuaWidgetClass<CBaseLuaInputField>("inputfield", 1);
    field->SetValue(luaL_checkstring(L, 2));
    return 0;
}

int CBaseLuaInputField::LuaGet(lua_State *L)
{
    CBaseLuaInputField *field = CheckLuaWidgetClass<CBaseLuaInputField>("inputfield", 1);
    
    if (field->GetType() == "string")
        lua_pushstring(L, field->GetValue());
    else if (field->GetType() == "number")
        lua_pushinteger(L, atoi(field->GetValue()));
    else if (field->GetType() == "float")
        lua_pushnumber(L, atof(field->GetValue()));
    
    return 1;
}

int CBaseLuaInputField::LuaSetLabelWidth(lua_State *L)
{
    CBaseLuaInputField *field = CheckLuaWidgetClass<CBaseLuaInputField>("inputfield", 1);
    int width = luaL_checkint(L, 2);
    
    field->SetLabelWidth(width);
    field->CoreUpdateLabelWidth();
    
    return 0;
}
