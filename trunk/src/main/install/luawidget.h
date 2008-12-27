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

#ifndef LUAWIDGET_H
#define LUAWIDGET_H

#include <string>
#include "main/lua/luaclass.h"

struct lua_State;

class CBaseLuaWidget
{
    std::string m_Title;
    const char *m_szLuaType;
    bool m_bEnabled;
    
    virtual void CoreUpdateLanguage(void) {}
    virtual void CoreSetTitle(void) = 0;
    virtual void CoreActivateWidget(void) = 0;
    virtual void CoreSetEnable(bool e) = 0;

protected:
    CBaseLuaWidget(const char *title) : m_szLuaType(0), m_bEnabled(true) { if (title && *title) m_Title = title; }
    CBaseLuaWidget(void) : m_szLuaType(0), m_bEnabled(true) {}
    
    void LuaDataChanged(void);
    const std::string &GetTitle(void) const { return m_Title; }

public:
    virtual ~CBaseLuaWidget(void) { };
    
    void UpdateLanguage(void) { CoreSetTitle(); CoreUpdateLanguage(); }
    void SetTitle(const std::string &title) { m_Title = title; CoreSetTitle(); }
    void Init(const char *type) { m_szLuaType = type; CoreSetTitle(); }
    bool Check(void);
    void ActivateWidget(void) { CoreActivateWidget(); }
    void SetEnable(bool e) { m_bEnabled = e; CoreSetEnable(e); }
    bool IsEnabled(void) const { return m_bEnabled; }
    
    static void LuaRegister(void);
    static int LuaEnable(lua_State *L);
};

template <typename C> C *CheckLuaWidgetClass(const char *type, int index)
{
    return dynamic_cast<C*>(NLua::CheckClassData<CBaseLuaWidget>(type, index));
}

#endif
