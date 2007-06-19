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
#include "main/lua/lua.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "luagroup.h"

// -------------------------------------
// Base Lua Group Class
// -------------------------------------

void CBaseLuaGroup::LuaRegister()
{
    NLua::RegisterClassFunction(LuaAddInput, "addinput", "group");
    NLua::RegisterClassFunction(LuaAddCheckbox, "addcheckbox", "group");
    NLua::RegisterClassFunction(LuaAddRadioButton, "addradiobutton", "group");
    NLua::RegisterClassFunction(LuaAddDirSelector, "adddirselector", "group");
    NLua::RegisterClassFunction(LuaAddCFGMenu, "addcfgmenu", "group");
    NLua::RegisterClassFunction(LuaAddMenu, "addmenu", "group");
    NLua::RegisterClassFunction(LuaAddImage, "addimage", "group");
    NLua::RegisterClassFunction(LuaAddProgressBar, "addprogressbar", "group");
}

int CBaseLuaGroup::LuaAddInput(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *label = luaL_optstring(L, 2, "");
    const char *desc = luaL_optstring(L, 3, "");
    int maxc = luaL_optint(L, 4, 1024);
    const char *val = luaL_optstring(L, 5, "");
    const char *type = luaL_optstring(L, 6, "string");
    
    if (strcmp(type, "string") && strcmp(type, "number") && strcmp(type, "float"))
        type = "string";
    
    NLua::CreateClass(group->CreateInputField(GetTranslation(label), GetTranslation(desc), val, maxc, type),
                      "inputfield");
    
    return 1;
}

int CBaseLuaGroup::LuaAddCheckbox(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_checkstring(L, 2);
    
    luaL_checktype(L, 3, LUA_TTABLE);
    int count = luaL_getn(L, 3);
    std::vector<std::string> l(count);
    
    for (int i=1; i<=count; i++)
    {
        lua_rawgeti(L, 3, i);
        const char *s = luaL_checkstring(L, -1);
        l[i-1] = s;
        lua_pop(L, 1);
    }
    
    NLua::CreateClass(group->CreateCheckbox(GetTranslation(desc), l), "checkbox");
    
    return 1;
}

int CBaseLuaGroup::LuaAddRadioButton(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_checkstring(L, 2);
    
    luaL_checktype(L, 3, LUA_TTABLE);
    int count = luaL_getn(L, 3);
    std::vector<std::string> l(count);

    
    for (int i=1; i<=count; i++)
    {
        lua_rawgeti(L, 3, i);
        const char *s = luaL_checkstring(L, -1);
        l[i-1] = s;
        lua_pop(L, 1);
    }
    
    NLua::CreateClass(group->CreateRadioButton(GetTranslation(desc), l), "radiobutton");

    return 1;
}

int CBaseLuaGroup::LuaAddDirSelector(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_optstring(L, 2, "");
    const char *val = luaL_optstring(L, 3, getenv("HOME"));

    NLua::CreateClass(group->CreateDirSelector(GetTranslation(desc), val), "dirselector");
    
    return 1;
}

int CBaseLuaGroup::LuaAddCFGMenu(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_optstring(L, 2, "");
    
    NLua::CreateClass(group->CreateCFGMenu(GetTranslation(desc)), "configmenu");
    
    return 1;
}

int CBaseLuaGroup::LuaAddMenu(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_checkstring(L, 2);
    
    luaL_checktype(L, 3, LUA_TTABLE);
    int count = luaL_getn(L, 3);
    std::vector<std::string> l(count);
    
    for (int i=1; i<=count; i++)
    {
        lua_rawgeti(L, 3, i);
        const char *s = luaL_checkstring(L, -1);
        l[i-1] = s;
        lua_pop(L, 1);
    }
    
    NLua::CreateClass(group->CreateMenu(GetTranslation(desc), l), "menu");
    
    return 1;
}

int CBaseLuaGroup::LuaAddImage(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_checkstring(L, 2);
    const char *file = luaL_checkstring(L, 3);

    NLua::CreateClass(group->CreateImage(GetTranslation(desc), file), "image");
    
    return 1;
}

int CBaseLuaGroup::LuaAddProgressBar(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_optstring(L, 2, "");

    NLua::CreateClass(group->CreateProgressBar(GetTranslation(desc)), "progressbar");
    
    return 1;
}
