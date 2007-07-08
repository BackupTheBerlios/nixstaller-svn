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

#ifndef BASE_SCREEN_H
#define BASE_SCREEN_H

class CBaseLuaGroup;

class CBaseScreen
{
    std::string m_Title;
    
    typedef std::vector<CBaseLuaGroup *> TLuaGroupList;
    TLuaGroupList m_LuaGroupList;
    
    bool CallLuaBoolFunc(const char *func, bool def);
    
    virtual CBaseLuaGroup *CreateGroup(void) = 0;
    virtual void CoreUpdateLanguage(void) = 0;

protected:
    virtual bool CoreBack(void) { return CallLuaBoolFunc("back", true); }
    virtual bool CoreNext(void) { return CallLuaBoolFunc("next", true); }
    virtual void CoreActivate(void);

    const std::string &GetTitle(void) { return m_Title; }

public:
    CBaseScreen(const std::string &title) : m_Title(title) { }
    virtual ~CBaseScreen(void) { }
    
    void UpdateLanguage(void);
    bool CanActivate(void) { return CallLuaBoolFunc("canactivate", true); }
    void Activate(void) { CoreActivate(); }
    bool Back(void) { return CoreBack(); }
    bool Next(void) { return CoreNext(); }
    
    static void LuaRegister(void);
    static int LuaAddGroup(lua_State *L);
};

#endif
