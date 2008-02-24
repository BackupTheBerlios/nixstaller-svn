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

#ifndef LUAPROGRESSDIALOG_H
#define LUAPROGRESSDIALOG_H

#include <vector>

class CBaseLuaProgressDialog
{
public:
    typedef std::vector<std::string> TStepList;
    
private:
    TStepList::size_type m_CurrentStep;
    TStepList m_StepList;
    int m_iFunctionRef;
    
    virtual void CoreSetNextStep(void) = 0;
    virtual void CoreEnableSecProgBar(bool enable) = 0;
    virtual void CoreSetSecTitle(const char *title) = 0;
    virtual void CoreSetSecProgress(int progress) = 0;
    virtual void CoreRun(void) = 0;
    
protected:
    TStepList::size_type GetCurrentStep(void) const { return m_CurrentStep; }
    TStepList &GetStepList(void) { return m_StepList; }
    void CallFunction(void);
    
public:
    CBaseLuaProgressDialog(const TStepList &l, int ref) : m_CurrentStep(0), m_StepList(l), m_iFunctionRef(ref) { }
    virtual ~CBaseLuaProgressDialog(void) { }
    
    void Run(void) { CoreRun(); }
    
    static void LuaRegister(void);
    
    static int LuaNextStep(lua_State *L);
    static int LuaEnableSecProgBar(lua_State *L);
    static int LuaSetSecTitle(lua_State *L);
    static int LuaSetSecProgress(lua_State *L);
};

#endif

