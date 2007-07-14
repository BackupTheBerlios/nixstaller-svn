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

#ifndef LUAINPUT_H
#define LUAINPUT_H

#include <string>
#include "luawidget.h"

struct lua_State;

class CBaseLuaInputField: public CBaseLuaWidget
{
    std::string m_szType;
    int m_iLabelWidth;
    
    virtual const char *CoreGetValue(void) = 0;
    virtual void CoreUpdateLabelWidth(void) = 0;
    
protected:
    const std::string &GetType(void) { return m_szType; };

public:
    CBaseLuaInputField(const char *t);
    
    const char *GetValue(void) { return CoreGetValue(); }
    void SetLabelWidth(int w) { m_iLabelWidth = w; CoreUpdateLabelWidth(); }

    int GetLabelWidth(void) const { return m_iLabelWidth; };

    static void LuaRegister(void);
    
    static int LuaGet(lua_State *L);
    static int LuaSetLabelWidth(lua_State *L);
};

#endif
