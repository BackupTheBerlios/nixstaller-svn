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
#include "luacheckbox.h"

// -------------------------------------
// Base Lua Checkbox Class
// -------------------------------------

void CBaseLuaCheckbox::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaCheckbox::LuaGet, "get", "checkbox");
    NLua::RegisterClassFunction(CBaseLuaCheckbox::LuaSet, "set", "checkbox");
}

int CBaseLuaCheckbox::LuaGet(lua_State *L)
{
    CBaseLuaCheckbox *box = NLua::CheckClassData<CBaseLuaCheckbox>("checkbox", 1);
    
    if (lua_isstring(L, 2))
    {
        if (lua_isnumber(L, 2))
        {
            TSTLVecSize n = SafeConvert<TSTLVecSize>(lua_tointeger(L, 2)) - 1; // Convert to 0-size range
            luaL_argcheck(L, ((n >= 0) && (n < box->m_Options.size())), 2,
                          "Tried to access non existing checkbutton entry");
            lua_pushboolean(L, box->Enabled(n));
        }
        else
        {
            const char *arg = lua_tostring(L, 2);
            luaL_argcheck(L, std::find(box->m_Options.begin(), box->m_Options.end(), arg) != box->m_Options.end(),
                          2, "Tried to access non existing checkbutton entry");
            lua_pushboolean(L, box->Enabled(arg));
        }
    }
    else
        luaL_typerror(L, 2, "Number or String");
    
    return 1;
}

int CBaseLuaCheckbox::LuaSet(lua_State *L)
{
    CBaseLuaCheckbox *box = NLua::CheckClassData<CBaseLuaCheckbox>("checkbox", 1);
    int args = lua_gettop(L);
    
    luaL_checktype(L, args, LUA_TBOOLEAN);
    bool e = lua_toboolean(L, args);

    TSTLVecSize size = box->m_Options.size();
    
    for (int i=2; i<args; i++)
    {
        TSTLVecSize n = SafeConvert<TSTLVecSize>(luaL_checkint(L, i)) - 1; // Convert to 0-size range
        luaL_argcheck(L, ((n >= 0) && (n < size)), i, "Tried to set non existing checkbutton entry");
        box->Enable(n, e);
    }
    
    return 0;
}
