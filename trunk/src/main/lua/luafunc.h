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

#ifndef LUA_FUNC_H
#define LUA_FUNC_H

#include "lua.h"

namespace NLua {

class CLuaFunc
{
    bool m_bOK;
    int m_iPushedArgs, m_iReturnedArgs;
    int m_iFuncIndex, m_iRetStartIndex;
    
    void PopRet(void);
    void CheckSelf(void);
    
public:
    CLuaFunc(const char *func, const char *tab=NULL);
    CLuaFunc(const char *func, const char *type, void *prvdata);
    ~CLuaFunc(void);

    CLuaFunc &operator <<(const std::string &arg);
    CLuaFunc &operator <<(int arg);
    CLuaFunc &operator <<(bool arg);
    CLuaFunc &operator >>(std::string &out);
    CLuaFunc &operator >>(int &out);
    CLuaFunc &operator >>(bool &arg);
    int operator ()(int ret=LUA_MULTRET);
    operator void*(void) { static int dummy; return (m_bOK) ? &dummy : 0; }
};

void RegisterFunction(lua_CFunction f, const char *name, const char *tab=NULL, void *data=NULL);
void RegisterClassFunction(lua_CFunction f, const char *name, const char *type, void *data=NULL);

}

#endif
