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
    virtual void CoreActivate(void);

    const std::string &GetTitle(void) const { return m_Title; }
    // This function is not called by the destructor, because in some frontends a lua group is part of the gui and will
    // be automaticly deleted
    void DeleteGroups(void);

public:
    CBaseScreen(const std::string &title) : m_Title(title) { }
    virtual ~CBaseScreen(void) { }
    
    void UpdateLanguage(void);
    bool CanActivate(void) { return CallLuaBoolFunc("canactivate", true); }
    void Activate(void) { CoreActivate(); }
    void Update(void);
    bool Back(void) { return CallLuaBoolFunc("back", true); }
    bool Next(void) { return CallLuaBoolFunc("next", true); }
    
    static void LuaRegister(void);
    static int LuaAddGroup(lua_State *L);
    static int LuaAddScreenEnd(lua_State *L);
};

#endif
