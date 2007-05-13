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

#ifndef LUA_TABLE_H
#define LUA_TABLE_H

#include <assert.h>
#include "lua.h"

namespace NLua {

class CLuaTable
{
    bool m_bOK, m_bClosed;
    int m_iTabIndex;
    
    void GetTable(const std::string &tab, int index);
    void CheckSelf(void);
    
public:
    class CReturn
    {
        friend class CLuaTable;
        
        int m_iIndexType;
        std::string m_Index;
        int m_iIndex;
        int m_iTabIndex;
        
        CReturn(const std::string &index, int tabind) : m_iIndexType(LUA_TSTRING), m_Index(index), m_iTabIndex(tabind) { }
        CReturn(int index, int tabind) : m_iIndexType(LUA_TNUMBER), m_iIndex(index), m_iTabIndex(tabind) { }
        
        void PushIndex(void);
        
    public:
        void operator <<(const std::string &val);
        void operator <<(int val);
        void operator >>(std::string &val);
        void operator >>(int &val);
    };
    
    CLuaTable(const char *var, const char *tab=NULL);
    CLuaTable(const char *var, const char *type, void *prvdata);
    ~CLuaTable(void) { assert(m_bClosed); }
    
    void Open(const char *var, const char *tab=NULL);
    void Open(const char *var, const char *type, void *prvdata);
    void Close(void);
    int Size(void) const { return luaL_getn(LuaState, m_iTabIndex); }
    
    CReturn operator [](const std::string &index) { CheckSelf(); return CReturn(index, m_iTabIndex); }
    CReturn operator [](int index) { CheckSelf(); return CReturn(index, m_iTabIndex); }
    operator void*(void) { static int ret; return (m_bOK) ? &ret : 0; }
};

}

#endif
