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

#include <algorithm>
#include "main/main.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "luacheckbox.h"

// -------------------------------------
// Base Lua Checkbox Class
// -------------------------------------

void CBaseLuaCheckbox::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaCheckbox::LuaGet, "get", "checkbox");
    NLua::RegisterClassFunction(CBaseLuaCheckbox::LuaSet, "set", "checkbox");
    NLua::RegisterClassFunction(CBaseLuaCheckbox::LuaAdd, "add", "checkbox");
    NLua::RegisterClassFunction(CBaseLuaCheckbox::LuaDel, "del", "checkbox");
    NLua::RegisterClassFunction(CBaseLuaCheckbox::LuaClear, "clear", "checkbox");
}

int CBaseLuaCheckbox::LuaGet(lua_State *L)
{
    CBaseLuaCheckbox *box = CheckLuaWidgetClass<CBaseLuaCheckbox>("checkbox", 1);
    
    if (box->m_Options.empty())
    {
        lua_pushnil(L);
        return 1;
    }
    
    int vartype = lua_type(L, 2);
    TSTLVecSize n = 0;
    if (vartype == LUA_TNUMBER)
        n = SafeConvert<TSTLVecSize>(lua_tointeger(L, 2)) - 1; // Convert to 0-size range
    else if (vartype == LUA_TSTRING)
        n = std::distance(box->m_Options.begin(),
                          std::find(box->m_Options.begin(), box->m_Options.end(), lua_tostring(L, 2)));
    else
        luaL_typerror(L, 2, "Number or String");

    luaL_argcheck(L, ((n >= 0) && (n < box->m_Options.size())), 2, "Tried to access non existing checkbutton entry");
    lua_pushboolean(L, box->Enabled(n));
    
    return 1;
}

int CBaseLuaCheckbox::LuaSet(lua_State *L)
{
    CBaseLuaCheckbox *box = CheckLuaWidgetClass<CBaseLuaCheckbox>("checkbox", 1);
    
    if (box->m_Options.empty())
        luaL_error(L, "Tried to select entry in empty checkbox widget.");

    int args = lua_gettop(L);
    
    luaL_checktype(L, args, LUA_TBOOLEAN);
    bool e = lua_toboolean(L, args);

    TSTLVecSize size = box->m_Options.size();
    
    for (int i=2; i<args; i++)
    {
        TSTLVecSize n = 0;
        int vartype = lua_type(L, i);
        if (vartype == LUA_TNUMBER)
            n = SafeConvert<TSTLVecSize>(lua_tointeger(L, i)) - 1; // Convert to 0-size range
        else if (vartype == LUA_TSTRING)
            n = std::distance(box->m_Options.begin(),
                              std::find(box->m_Options.begin(), box->m_Options.end(), lua_tostring(L, i)));
        else
            luaL_typerror(L, i, "Number or String");

        luaL_argcheck(L, ((n >= 0) && (n < size)), i, "Tried to set non existing checkbutton entry");
        box->Enable(n, e);
    }
    
    return 0;
}

int CBaseLuaCheckbox::LuaAdd(lua_State *L)
{
    CBaseLuaCheckbox *box = CheckLuaWidgetClass<CBaseLuaCheckbox>("checkbox", 1);
    const char *label = luaL_checkstring(L, 2);
    int pos = luaL_optint(L, 3, -1);
    
    TSTLVecSize n;
    if (pos < 0)
        n = box->m_Options.size();
    else
        n = SafeConvert<TSTLVecSize>(pos) - 1;
    
    if (n >= box->m_Options.size())
        box->m_Options.push_back(label);
    else
        box->m_Options.insert(box->m_Options.begin() + n, label);
    
    box->AddOption(label, n);
    
    return 0;
}

int CBaseLuaCheckbox::LuaDel(lua_State *L)
{
    CBaseLuaCheckbox *box = CheckLuaWidgetClass<CBaseLuaCheckbox>("checkbox", 1);
    
    if (box->m_Options.empty())
        luaL_error(L, "Can't delete entry; checkbox is empty.");
        
    int vartype = lua_type(L, 2);
    TSTLVecSize n = 0;
    
    if (vartype == LUA_TNUMBER)
        n = SafeConvert<TSTLVecSize>(lua_tointeger(L, 2)) - 1;
    else if (vartype == LUA_TSTRING)
        n = std::distance(box->m_Options.begin(),
                          std::find(box->m_Options.begin(), box->m_Options.end(), lua_tostring(L, 2)));
    else
        luaL_typerror(L, 2, "Number or String");

    luaL_argcheck(L, ((n >= 0) && (n < box->m_Options.size())), 2, "Tried to delete non existing checkbutton entry");

    box->DelOption(n);

    box->m_Options.erase(box->m_Options.begin() + n);
    return 0;
}

int CBaseLuaCheckbox::LuaClear(lua_State *L)
{
    CBaseLuaCheckbox *box = CheckLuaWidgetClass<CBaseLuaCheckbox>("checkbox", 1);
    
    TSTLVecSize size = box->m_Options.size();
    while (size)
    {
        box->DelOption(size-1);
        box->m_Options.pop_back();
        size--;
    }
    
    return 0;
}
