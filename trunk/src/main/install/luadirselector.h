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

#ifndef LUADIRSELECTOR_H
#define LUADIRSELECTOR_H

#include "luawidget.h"

struct lua_State;

class CBaseLuaDirSelector: public CBaseLuaWidget
{
    virtual const char *LuaType(void) const { return "dirselector"; }
    virtual const char *GetDir(void) = 0;
    virtual void SetDir(const char *dir) = 0;

public:
    static void LuaRegister(void);
    static int LuaGet(lua_State *L);
    static int LuaSet(lua_State *L);
};


#endif
