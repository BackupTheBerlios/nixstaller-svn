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

#ifndef LUAWIDGET_H
#define LUAWIDGET_H

#include <string>

struct lua_State;

class CBaseLuaWidget
{
    std::string m_Title;
    const char *m_szLuaType;
    
    virtual void CoreUpdateLanguage(void) {}
    virtual void CoreSetTitle(void) = 0;
    
protected:
    CBaseLuaWidget(const char *title) { if (title && *title) m_Title = title; }
    
    void LuaDataChanged(void);
    std::string &GetTitle(void) { return m_Title; }

public:
    CBaseLuaWidget(void) : m_szLuaType(0) {}
    virtual ~CBaseLuaWidget(void) { };
    
    void UpdateLanguage(void) { CoreUpdateLanguage(); }
    void SetTitle(const std::string &title) { m_Title = title; CoreSetTitle(); }
    void Init(const char *type) { m_szLuaType = type; }
    bool Check(void);
};

#endif
