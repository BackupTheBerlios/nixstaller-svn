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

#include "main/lua/luafunc.h"
#include "luawidget.h"

// -------------------------------------
// Base Lua Widget Class
// -------------------------------------

CBaseLuaWidget::CBaseLuaWidget(const char *title) : m_szLuaType(0), m_bEnabled(true), m_bLabelBold(false),
                                                    m_bLabelItalic(false), m_eLabelSize(LABEL_NORMAL)
{
    if (title && *title)
        m_Title = title;
}

CBaseLuaWidget::CBaseLuaWidget() : m_szLuaType(0), m_bEnabled(true), m_bLabelBold(false),
                                   m_bLabelItalic(false), m_eLabelSize(LABEL_NORMAL)
{
}

void CBaseLuaWidget::LuaDataChanged()
{
    NLua::CLuaFunc func("datachanged", m_szLuaType, this);
    if (func)
    {
        NLua::PushClass(m_szLuaType, this);
        func.PushData();
        func(0);
    }
}

bool CBaseLuaWidget::Check()
{
    bool ret = true;
    
    NLua::CLuaFunc func("verify", m_szLuaType, this);
    if (func)
    {
        NLua::PushClass(m_szLuaType, this);
        func.PushData();

        if (func(1) > 0)
            func >> ret;
        else
            ret = false;
    }
    
    return ret;
}

void CBaseLuaWidget::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaWidget::LuaEnable, "enable", "widget");
    NLua::RegisterClassFunction(CBaseLuaWidget::LuaSetLabel, "setlabel", "widget");
    NLua::RegisterClassFunction(CBaseLuaWidget::LuaSetLabelAttr, "setlabelattr", "widget");
}

int CBaseLuaWidget::LuaEnable(lua_State *L)
{
    CBaseLuaWidget *widget = CheckLuaWidgetClass<CBaseLuaWidget>("widget", 1);
    widget->SetEnable(NLua::LuaToBool(2));
    return 0;
}

int CBaseLuaWidget::LuaSetLabel(lua_State *L)
{
    CBaseLuaWidget *widget = CheckLuaWidgetClass<CBaseLuaWidget>("widget", 1);
    widget->SetTitle(luaL_checkstring(L, 2));
    return 0;
}

int CBaseLuaWidget::LuaSetLabelAttr(lua_State *L)
{
    CBaseLuaWidget *widget = CheckLuaWidgetClass<CBaseLuaWidget>("widget", 1);
    const char *attr = luaL_checkstring(L, 2);
    luaL_argcheck(L, (!strcmp(attr, "bold") || !strcmp(attr, "italic") || !strcmp(attr, "size")),
                  2, "Should be bold, italic or size");
    
    if (!strcmp(attr, "bold"))
        widget->m_bLabelBold = NLua::LuaToBool(3);
    else if (!strcmp(attr, "italic"))
        widget->m_bLabelItalic = NLua::LuaToBool(3);
    else // size
    {
        const char *size = luaL_checkstring(L, 3);
        luaL_argcheck(L, (!strcmp(size, "small") || !strcmp(size, "normal") || !strcmp(size, "big")),
                      3, "Should be small, normal or big");

        if (!strcmp(size, "small"))
            widget->m_eLabelSize = LABEL_SMALL;
        else if (!strcmp(size, "normal"))
            widget->m_eLabelSize = LABEL_NORMAL;
        else // big
            widget->m_eLabelSize = LABEL_BIG;
    }
    
    widget->CoreSetTitle(); // Update
    
    return 0;
}
