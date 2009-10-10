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

#ifndef LUA_TABLE_H
#define LUA_TABLE_H

#include "main/main.h"
#include "lua.h"

#include <string>
#include <vector>

namespace NLua {

class CLuaTable
{
    bool m_bOK;
    int m_iTabRef;
    
    void GetTable(const std::string &tab, int index);
    void GetTable(int ref, int index);
    void CheckSelf(void);
    
public:
    class CReturn
    {
        friend class CLuaTable;
        
        int m_iIndexRef;
        int m_iTabRef;
        
        CReturn(const std::string &index, int tab);
        CReturn(int index, int tab);
        
    public:
        ~CReturn(void);
        
        void GetTable(void);
        void SetTable(void);

        void operator <<(const std::string &val);
        void operator <<(const char *val);
        void operator <<(double val);
        void operator <<(int val);
        void operator <<(bool val);
        void operator <<(const CLuaTable &tab);
        template <typename C> void operator <<(const std::vector<C> &vec)
        {
            CLuaTable t(vec.begin(), vec.end());
            operator <<(t);
        }
        
        void operator >>(std::string &val);
        void operator >>(const char *&val);
        void operator >>(int &val);
        void operator >>(bool &val);
        void operator >>(CLuaTable &tab);
        template <typename C> void operator >>(std::vector<C> &out)
        {
            CLuaTable t;
            operator >>(t);
            t.ToVec(out);
        }
        operator void *(void);
    };
    
    CLuaTable(const char *var, const char *tab=NULL);
    CLuaTable(const char *var, const char *type, void *prvdata);
    CLuaTable(int index);
    CLuaTable(void); // Creates new table
    CLuaTable(const CLuaTable &lt);
    template <typename IT> CLuaTable(IT start, IT end)
    {
        New();
        int n = 1;
        while (start != end)
        {
            // Although setting it via CReturn may be less efficient,
            // this way we can set all types of data without duplicating
            // stuff.
            CReturn(n, m_iTabRef) << *start;
            start++;
            n++;
        }
    }
    ~CLuaTable(void) { Close(); }
    
    void New(void);
    void Open(const char *var, const char *tab=NULL);
    void Open(const char *var, const char *type, void *prvdata);
    void Open(int index);
    void Close(void);
    int Size(void);
    bool Next(std::string &key);
    void GetTable(void) const;
    
    template <typename C> void ToVec(std::vector<C> &out)
    {
        const int size = Size();
        for (int n=1; n<=size; n++)
        {
            C v;
            CReturn(n, m_iTabRef) >> v;
            out.push_back(v);
        }
    }
    
    CReturn operator [](const std::string &index) { CheckSelf(); return CReturn(index, m_iTabRef); }
    CReturn operator [](int index) { CheckSelf(); return CReturn(index, m_iTabRef); }
    operator void *(void) { static int ret; return (m_bOK) ? &ret : 0; }
    bool OK(void) const { return m_bOK; }
};


}

#endif
