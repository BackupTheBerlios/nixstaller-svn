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
#include "luacfgmenu.h"

// -------------------------------------
// Base Lua config menu Class
// -------------------------------------

CBaseLuaCFGMenu::~CBaseLuaCFGMenu()
{
    for (TVarType::iterator it=m_Variables.begin(); it!=m_Variables.end(); it++)
        delete it->second;
}

void CBaseLuaCFGMenu::AddVar(const char *name, const char *desc, const char *val, EVarType type, TOptionsType *l)
{
    if (m_Variables[name])
    {
        delete m_Variables[name];
        m_Variables[name] = NULL;
    }
    
    const char *value = val;
    
    if (l)
        std::sort(l->begin(), l->end());
    
    if (!value || !*value)
    {
        switch (type)
        {
            case TYPE_DIR:
                value = "/";
                break;
            case TYPE_STRING:
                value = "";
                break;
            case TYPE_LIST:
                if (l)
                    value = l->front().c_str();
                else
                    value = "";
                break;
            case TYPE_BOOL:
                value = "Disable";
                break;
        }
    }
    
    m_Variables[name] = new entry_s(value, desc, type);
    
    if (l)
    {
        m_Variables[name]->options = *l;
    }
    
    CoreAddVar(name);
}

void CBaseLuaCFGMenu::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaCFGMenu::LuaGet, "get", "configmenu");
    NLua::RegisterClassFunction(CBaseLuaCFGMenu::LuaAddDir, "adddir", "configmenu");
    NLua::RegisterClassFunction(CBaseLuaCFGMenu::LuaAddString, "addstring", "configmenu");
    NLua::RegisterClassFunction(CBaseLuaCFGMenu::LuaAddList, "addlist", "configmenu");
    NLua::RegisterClassFunction(CBaseLuaCFGMenu::LuaAddBool, "addbool", "configmenu");
}

int CBaseLuaCFGMenu::LuaAddDir(lua_State *L)
{
    CBaseLuaCFGMenu *menu = NLua::CheckClassData<CBaseLuaCFGMenu>("configmenu", 1);
    const char *var = luaL_checkstring(L, 2);
    const char *desc = luaL_checkstring(L, 3);
    const char *val = luaL_optstring(L, 4, getenv("HOME"));

    menu->AddVar(var, GetTranslation(desc), val, TYPE_DIR);
    
    return 0;
}

int CBaseLuaCFGMenu::LuaAddString(lua_State *L)
{
    CBaseLuaCFGMenu *menu = NLua::CheckClassData<CBaseLuaCFGMenu>("configmenu", 1);
    const char *var = luaL_checkstring(L, 2);
    const char *desc = luaL_checkstring(L, 3);
    const char *val = lua_tostring(L, 4);

    menu->AddVar(var, GetTranslation(desc), val, TYPE_STRING);
    
    return 0;
}

int CBaseLuaCFGMenu::LuaAddList(lua_State *L)
{
    CBaseLuaCFGMenu *menu = NLua::CheckClassData<CBaseLuaCFGMenu>("configmenu", 1);
    const char *var = luaL_checkstring(L, 2);
    const char *desc = luaL_checkstring(L, 3);
    
    luaL_checktype(L, 4, LUA_TTABLE);
    int count = luaL_getn(L, 4);
    TOptionsType l;
    
    for (int i=1; i<=count; i++)
    {
        lua_rawgeti(L, 4, i);
        const char *s = luaL_checkstring(L, -1);
        l.push_back(s);
        lua_pop(L, 1);
    }
    
    if (l.empty())
        luaL_error(L, "Empty list given");

    const char *val = lua_tostring(L, 5);
    
    menu->AddVar(var, GetTranslation(desc), val, TYPE_LIST, &l);
    
    return 0;
}

int CBaseLuaCFGMenu::LuaAddBool(lua_State *L)
{
    CBaseLuaCFGMenu *menu = NLua::CheckClassData<CBaseLuaCFGMenu>("configmenu", 1);
    const char *var = luaL_checkstring(L, 2);
    const char *desc = luaL_checkstring(L, 3);
    bool val = (lua_isboolean(L, 4)) ? lua_toboolean(L, 4) : false;

    TOptionsType l;
    l.push_back("Disable");
    l.push_back("Enable");
    
    menu->AddVar(var, GetTranslation(desc), menu->BoolToStr(val), TYPE_BOOL, &l);
    
    return 0;
}

int CBaseLuaCFGMenu::LuaGet(lua_State *L)
{
    CBaseLuaCFGMenu *menu = NLua::CheckClassData<CBaseLuaCFGMenu>("configmenu", 1);
    const char *var = luaL_checkstring(L, 2);
    
    if (menu->m_Variables[var])
    {
        if (menu->m_Variables[var]->type == TYPE_BOOL)
            lua_pushboolean(L, (menu->m_Variables[var]->val == "Enable") ? true : false);
        else
            lua_pushstring(L, menu->m_Variables[var]->val.c_str());
    }
    else
        luaL_error(L, "No variabele %s found in menu\n", var);
    
    return 1;
}
