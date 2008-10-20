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
#include "basescreen.h"
#include "luagroup.h"
#include "main/lua/lua.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"


// -------------------------------------
// Base Install Class
// -------------------------------------

bool CBaseScreen::CallLuaBoolFunc(const char *func, bool def)
{
    NLua::CLuaFunc luafunc(func, "screen", this);
    bool ret = def;
    
    if (luafunc)
    {
        NLua::PushClass("screen", this);
        luafunc.PushData(); // Add 'self' to function argument list
        
        if (luafunc(1) > 0)
            luafunc >> ret;
    }
        
    return ret;
}

void CBaseScreen::CoreActivate()
{
    NLua::CLuaFunc func("activate", "screen", this);
    
    if (func)
    {
        NLua::PushClass("screen", this);
        func.PushData(); // Add 'self' to function argument list
        func(0);
    }
}

void CBaseScreen::DeleteGroups()
{
    debugline("Deleting %u groups...\n", m_LuaGroupList.size());
    
    while (!m_LuaGroupList.empty())
    {
        delete m_LuaGroupList.back();
        m_LuaGroupList.pop_back();
    }
}

void CBaseScreen::UpdateLanguage()
{
    CoreUpdateLanguage();
    
    for (TLuaGroupList::iterator it=m_LuaGroupList.begin(); it!=m_LuaGroupList.end(); it++)
        (*it)->UpdateLanguage();
}

void CBaseScreen::Update()
{
    NLua::CLuaFunc luafunc("update", "screen", this);
    if (luafunc)
        luafunc(0);
}

void CBaseScreen::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseScreen::LuaAddGroup, "addgroup", "screen");
    NLua::RegisterClassFunction(CBaseScreen::LuaAddScreenEnd, "addscreenend", "screen");
}

int CBaseScreen::LuaAddGroup(lua_State *L)
{
    CBaseScreen *screen = NLua::CheckClassData<CBaseScreen>("screen", 1);
    CBaseLuaGroup *group = screen->CreateGroup();
    screen->m_LuaGroupList.push_back(group);
    NLua::CreateClass(group, "group");
    return 1;
}

int CBaseScreen::LuaAddScreenEnd(lua_State *L)
{
    CBaseScreen *screen = NLua::CheckClassData<CBaseScreen>("screen", 1);
    if (!screen->m_LuaGroupList.empty())
        screen->m_LuaGroupList.back()->SetScreenEnds(true);
    return 0;
}
