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

#ifndef LUAMENU_H
#define LUAMENU_H

#include "luawidget.h"

struct LuaState;

class CBaseLuaMenu: virtual public CBaseLuaWidget
{
protected:
    typedef std::vector<std::string> TOptions;
    
private:
    TOptions m_Options;
    
    virtual std::string Selection(void) = 0;
    virtual void Select(TSTLVecSize n) = 0;
    
protected:
    CBaseLuaMenu(const TOptions &opts) : m_Options(opts) {}
    
    TOptions &GetOptions(void) { return m_Options; }
    
public:
    virtual ~CBaseLuaMenu(void) {}

    static void LuaRegister(void);
    static int LuaGet(lua_State *L);
    static int LuaSet(lua_State *L);
};

#endif
