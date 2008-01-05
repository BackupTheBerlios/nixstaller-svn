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
#include "main/lua/lua.h"
#include "main/lua/luafunc.h"
#include "luagroup.h"
#include "luacheckbox.h"
#include "luacfgmenu.h"
#include "luadirselector.h"
#include "luaimage.h"
#include "luainput.h"
#include "lualabel.h"
#include "luamenu.h"
#include "luaprogressbar.h"
#include "luaradiobutton.h"
#include "luatextfield.h"
#include "luawidget.h"

// -------------------------------------
// Base Lua Group Class
// -------------------------------------

void CBaseLuaGroup::AddWidget(CBaseLuaWidget *w, const char *type)
{
    w->Init(type);
    NLua::CreateClass(w, type);
    m_WidgetList.push_back(w);
}

void CBaseLuaGroup::DeleteWidgets()
{
    debugline("Deleting %u widgets...\n", m_WidgetList.size());
    
    while (!m_WidgetList.empty())
    {
        delete m_WidgetList.back();
        m_WidgetList.pop_back();
    }
}

void CBaseLuaGroup::UpdateLanguage()
{
    for (TWidgetList::iterator it=m_WidgetList.begin(); it!=m_WidgetList.end(); it++)
        (*it)->UpdateLanguage();
}

bool CBaseLuaGroup::CheckWidgets()
{
    for (TWidgetList::iterator it=m_WidgetList.begin(); it!=m_WidgetList.end(); it++)
    {
        if (!(*it)->Check())
        {
            (*it)->ActivateWidget();
            return false;
        }
    }
    
    return true;
}

void CBaseLuaGroup::LuaRegister()
{
    // Keep these in sync with install.lua!
    NLua::RegisterClassFunction(LuaAddInput, "addinput", "group");
    NLua::RegisterClassFunction(LuaAddCheckbox, "addcheckbox", "group");
    NLua::RegisterClassFunction(LuaAddRadioButton, "addradiobutton", "group");
    NLua::RegisterClassFunction(LuaAddDirSelector, "adddirselector", "group");
    NLua::RegisterClassFunction(LuaAddCFGMenu, "addcfgmenu", "group");
    NLua::RegisterClassFunction(LuaAddMenu, "addmenu", "group");
    NLua::RegisterClassFunction(LuaAddImage, "addimage", "group");
    NLua::RegisterClassFunction(LuaAddProgressBar, "addprogressbar", "group");
    NLua::RegisterClassFunction(LuaAddTextField, "addtextfield", "group");
    NLua::RegisterClassFunction(LuaAddLabel, "addlabel", "group");
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
    
    group->AddWidget(group->CreateInputField(label, desc, val, maxc, type), "inputfield");
    
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
    
    group->AddWidget(group->CreateCheckbox(desc, l), "checkbox");
    
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
    
    group->AddWidget(group->CreateRadioButton(desc, l), "radiobutton");

    return 1;
}

int CBaseLuaGroup::LuaAddDirSelector(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_optstring(L, 2, "");
    const char *val = luaL_optstring(L, 3, getenv("HOME"));

    group->AddWidget(group->CreateDirSelector(desc, val), "dirselector");
    
    return 1;
}

int CBaseLuaGroup::LuaAddCFGMenu(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_optstring(L, 2, "");
    
    group->AddWidget(group->CreateCFGMenu(desc), "configmenu");
    
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
    
    group->AddWidget(group->CreateMenu(desc, l), "menu");
    
    return 1;
}

int CBaseLuaGroup::LuaAddImage(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *file = luaL_checkstring(L, 2);

    group->AddWidget(group->CreateImage(file), "image");
    
    return 1;
}

int CBaseLuaGroup::LuaAddProgressBar(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_optstring(L, 2, "");

    group->AddWidget(group->CreateProgressBar(desc), "progressbar");
    
    return 1;
}

int CBaseLuaGroup::LuaAddTextField(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_optstring(L, 2, "");
    bool wrap = false;
    
    if (lua_isboolean(L, 3))
        wrap = lua_toboolean(L, 3);

    group->AddWidget(group->CreateTextField(desc, wrap), "textfield");
    
    return 1;
}

int CBaseLuaGroup::LuaAddLabel(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_checkstring(L, 2);

    group->AddWidget(group->CreateLabel(desc), "label");
    
    return 1;
}
