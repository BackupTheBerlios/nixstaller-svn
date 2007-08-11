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
#include "luatextfield.h"

// -------------------------------------
// Base Lua Text Field Class
// -------------------------------------

void CBaseLuaTextField::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaTextField::LuaLoad, "load", "textfield");
    NLua::RegisterClassFunction(CBaseLuaTextField::LuaAddText, "add", "textfield");
    NLua::RegisterClassFunction(CBaseLuaTextField::LuaSetFollow, "follow", "textfield");
}

int CBaseLuaTextField::LuaLoad(lua_State *L)
{
    CBaseLuaTextField *field = NLua::CheckClassData<CBaseLuaTextField>("textfield", 1);
    field->Load(luaL_checkstring(L, 2));
    return 0;
}

int CBaseLuaTextField::LuaAddText(lua_State *L)
{
    CBaseLuaTextField *field = NLua::CheckClassData<CBaseLuaTextField>("textfield", 1);
    field->AddText(luaL_checkstring(L, 2));
    return 0;
}

int CBaseLuaTextField::LuaSetFollow(lua_State *L)
{
    CBaseLuaTextField *field = NLua::CheckClassData<CBaseLuaTextField>("textfield", 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    field->m_bFollow = lua_toboolean(L, 2);
    field->UpdateFollow();
    return 0;
}
