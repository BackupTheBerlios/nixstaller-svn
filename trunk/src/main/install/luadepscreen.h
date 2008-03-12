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

#ifndef LUADEPSCREEN_H
#define LUADEPSCREEN_H

#include <string>
#include <vector>

class CBaseLuaDepScreen
{
protected:
    struct SDepInfo
    {
        std::string name, description, problem;
        SDepInfo(const std::string &n, const std::string &d, const std::string &p) : name(n),
                                                                                     description(d),
                                                                                     problem(p) { }
    };
    
    typedef std::vector<SDepInfo> TDepList;
    
private:
    TDepList m_Dependencies;
    int m_iLuaCheckRef;
    
    virtual void CoreUpdateList(void) = 0;
    virtual void CoreRun(void) = 0;
    
protected:
    const TDepList &GetDepList(void) { return m_Dependencies; }
    void Refresh(void);
    const char *GetTitle(void) const;
    
public:
    CBaseLuaDepScreen(int f) : m_iLuaCheckRef(f) {}
    virtual ~CBaseLuaDepScreen(void) {}
    
    void Run(void) { Refresh(); CoreRun(); }
};


#endif
