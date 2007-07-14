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
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "luawidget.h"

// -------------------------------------
// Base Lua Widget Class
// -------------------------------------

CBaseLuaWidget::CBaseLuaWidget() : m_iCheckRef(LUA_NOREF)
{
}

bool CBaseLuaWidget::Check()
{
    bool ret = true;
    
    if (m_iCheckRef != LUA_NOREF)
    {
        NLua::CLuaFunc func(m_iCheckRef, LUA_REGISTRYINDEX);
        if (func)
        {
            if (func(1) > 0)
                func >> ret;
        }
    }
    
    return ret;
}

void CBaseLuaWidget::LuaRegisterCheck(const char *type)
{
    NLua::RegisterClassFunction(LuaSetCheck, "setcheck", type, const_cast<char *>(type));
}

int CBaseLuaWidget::LuaSetCheck(lua_State *L)
{
    const char *type = reinterpret_cast<const char *>(lua_touserdata(L, lua_upvalueindex(1)));
    CBaseLuaWidget *widget = NLua::CheckClassData<CBaseLuaWidget>(type, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    widget->m_iCheckRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}
