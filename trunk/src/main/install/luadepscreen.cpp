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
#include "main/install/luadepscreen.h"
#include "main/lua/luafunc.h"
#include "main/lua/luatable.h"

// -------------------------------------
// Lua Dependency Screen Class
// -------------------------------------

void CBaseLuaDepScreen::Refresh()
{
    NLua::CLuaFunc func(m_iLuaCheckRef, LUA_REGISTRYINDEX);
    if (func)
    {
        func(1);
        func.PopData();
        luaL_checktype(NLua::LuaState, -1, LUA_TTABLE);
        NLua::CLuaTable deps(-1);
        
        m_Dependencies.clear();
        
        if (deps)
        {
            std::string key;
            while (deps.Next(key))
            {
                deps[key].GetTable();
                luaL_checktype(NLua::LuaState, -1, LUA_TTABLE);
                NLua::CLuaTable tab(-1);
                
                if (tab)
                {
                    const char *d, *p;
                    tab["desc"] >> d;
                    tab["problem"] >> p;
                    m_Dependencies.push_back(SDepInfo(key, d, p));
                }
                
                lua_pop(NLua::LuaState, 1); // deps[key]
            }
        }
        
        lua_pop(NLua::LuaState, 1); // Func ret
        
#if 0
        int tab = lua_gettop(NLua::LuaState);
        int count = luaL_getn(NLua::LuaState, tab);
        
        m_Dependencies.clear();
        
        for (int i=1; i<=count; i++)
        {
            lua_rawgeti(NLua::LuaState, tab, i);
            luaL_checktype(NLua::LuaState, -1, LUA_TTABLE);
            NLua::CLuaTable tab(-1);
            if (tab)
            {
                const char *n, *d, *p;
                tab["name"] >> n;
                tab["desc"] >> d;
                tab["problem"] >> p;
                m_Dependencies.push_back(SDepInfo(n, d, p));
            }
        }
        lua_pop(NLua::LuaState, 1);
#endif
    }
    
    CoreUpdateList();
}

const char *CBaseLuaDepScreen::GetTitle() const
{
    return GetTranslation("Dependency title here...");
}
