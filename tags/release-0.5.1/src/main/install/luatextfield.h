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

#ifndef LUATEXTFIELD_H
#define LUATEXTFIELD_H

#include "luawidget.h"

struct lua_State;

class CBaseLuaTextField: virtual public CBaseLuaWidget
{
    bool m_bFollow;
    
    virtual void Load(const char *file) = 0;
    virtual void AddText(const char *text) = 0;
    virtual void ClearText(void) = 0;
    virtual void UpdateFollow(void) {}
    
protected:
    bool Follow(void) const { return m_bFollow; }
    int MaxSize(void) const { return 4 * 1024 * 1024; } // 4MB max
    int ClearSize(void) const { return 512; } // Bytes to remove when size exceeds max
    
public:
    CBaseLuaTextField(void) : m_bFollow(false) { }
    virtual ~CBaseLuaTextField(void) {}

    static void LuaRegister(void);
    static int LuaLoad(lua_State *L);
    static int LuaAddText(lua_State *L);
    static int LuaClearText(lua_State *L);
    static int LuaSetFollow(lua_State *L);
};


#endif
