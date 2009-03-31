/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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

#include <stdlib.h>

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

std::map<std::string, bool> CBaseLuaGroup::m_RegisteredWidgets;

void CBaseLuaGroup::AddWidget(CBaseLuaWidget *w, const char *type)
{
    if (m_RegisteredWidgets.find(type) == m_RegisteredWidgets.end())
    {
        m_RegisteredWidgets[type] = true;
        NLua::SetClassBase(type, "widget");
    }
    
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

bool CBaseLuaGroup::IsEnabled() const
{
    for (TWidgetList::const_iterator it=m_WidgetList.begin(); it!=m_WidgetList.end(); it++)
    {
        if ((*it)->IsEnabled())
            return true;
    }
    
    return false;
}

bool CBaseLuaGroup::CheckWidgets()
{
    for (TWidgetList::iterator it=m_WidgetList.begin(); it!=m_WidgetList.end(); it++)
    {
        if ((*it)->IsEnabled() && !(*it)->Check())
        {
            (*it)->ActivateWidget();
            return false;
        }
    }
    
    return true;
}

void CBaseLuaGroup::LuaRegister()
{
    // Keep these in sync with attinstall.lua!
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
    std::vector<std::string> l;
    std::vector<TSTLVecSize> e;
    
    if (lua_istable(L, 3))
    {
        int count = luaL_getn(L, 3);
        for (int i=1; i<=count; i++)
        {
            lua_rawgeti(L, 3, i);
            const char *s = luaL_checkstring(L, -1);
            l.push_back(s);
            lua_pop(L, 1);
        }
        
        if (lua_istable(L, 4))
        {
            count = luaL_getn(L, 4);
            for (int i=1; i<=count; i++)
            {
                lua_rawgeti(L, 4, i);
                int n = luaL_checkint(L, -1) - 1;
                luaL_argcheck(L, ((n >= 0) && (n < l.size())), 4, "Tried to set non existing checkbox entry");
                e.push_back(SafeConvert<TSTLVecSize>(n));
                lua_pop(L, 1);
            }
        }
    }

    group->AddWidget(group->CreateCheckbox(desc, l, e), "checkbox");
    
    return 1;
}

int CBaseLuaGroup::LuaAddRadioButton(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_checkstring(L, 2);
    std::vector<std::string> l;
    TSTLVecSize sel = 0;
    
    if (lua_istable(L, 3))
    {
        int count = luaL_getn(L, 3);
    
        for (int i=1; i<=count; i++)
        {
            lua_rawgeti(L, 3, i);
            const char *s = luaL_checkstring(L, -1);
            l.push_back(s);
            lua_pop(L, 1);
        }
        
        int e = luaL_optint(L, 4, 1) - 1;
        luaL_argcheck(L, ((e >= 0) && (e < l.size())), 4, "Tried to set non existing radiobutton entry");
        sel = SafeConvert<TSTLVecSize>(e);
    }
    
    group->AddWidget(group->CreateRadioButton(desc, l, sel), "radiobutton");

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
    
    std::vector<std::string> l;
    TSTLVecSize sel = 0;
    
    if (lua_istable(L, 3))
    {
        const int count = luaL_getn(L, 3);
        for (int i=1; i<=count; i++)
        {
            lua_rawgeti(L, 3, i);
            const char *s = luaL_checkstring(L, -1);
            l.push_back(s);
            lua_pop(L, 1);
        }
        
        int e = luaL_optint(L, 4, 1) - 1;
        luaL_argcheck(L, ((e >= 0) && (e < l.size())), 4, "Tried to select non existing menu entry");
        sel = SafeConvert<TSTLVecSize>(e);
    }
    
    group->AddWidget(group->CreateMenu(desc, l, sel), "menu");
    
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

    const char *size = "medium";
    if (lua_isstring(L, 4))
    {
        const char *s = lua_tostring(L, 4);
        if (!strcmp(s, "small"))
            size = "small";
        else if (!strcmp(s, "big"))
            size = "big";
    }
    
    group->AddWidget(group->CreateTextField(desc, wrap, size), "textfield");
    
    return 1;
}

int CBaseLuaGroup::LuaAddLabel(lua_State *L)
{
    CBaseLuaGroup *group = NLua::CheckClassData<CBaseLuaGroup>("group", 1);
    const char *desc = luaL_checkstring(L, 2);

    group->AddWidget(group->CreateLabel(desc), "label");
    
    return 1;
}
