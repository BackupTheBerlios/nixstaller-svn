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

#ifndef LUAPROGRESSBAR_H
#define LUAPROGRESSBAR_H

#include "luawidget.h"

struct lua_State;

class CBaseLuaProgressBar: virtual public CBaseLuaWidget
{
    virtual void SetProgress(float n) = 0;
    
public:
    virtual ~CBaseLuaProgressBar(void) {}

    static void LuaRegister(void);
    static int LuaSet(lua_State *L);
};


#endif
