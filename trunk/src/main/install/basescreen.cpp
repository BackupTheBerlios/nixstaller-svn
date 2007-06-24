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
#include "basescreen.h"
#include "main/lua/lua.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"


// -------------------------------------
// Base Install Class
// -------------------------------------

bool CBaseScreen::CallLuaBoolFunc(const char *func, bool def)
{
    NLua::CLuaFunc luafunc("func", "screen", this);
    bool ret = def;
    
    if (luafunc)
    {
        if (luafunc(1) > 0)
            luafunc >> ret;
    }
        
    return ret;
}

void CBaseScreen::CoreActivate()
{
    NLua::CLuaFunc func("activate", "screen", this);
    
    if (func)
        func(0);
}

void CBaseScreen::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseScreen::LuaAddGroup, "addgroup", "screen");
}

int CBaseScreen::LuaAddGroup(lua_State *L)
{
    CBaseScreen *screen = NLua::CheckClassData<CBaseScreen>("screen", 1);
    NLua::CreateClass(screen->CreateGroup(), "group");
    return 1;
}
