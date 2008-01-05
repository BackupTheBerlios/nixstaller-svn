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

#include "main/lua/luafunc.h"
#include "luawidget.h"

// -------------------------------------
// Base Lua Widget Class
// -------------------------------------

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
    }
    
    return ret;
}
