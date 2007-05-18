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

class CBaseScreen
{
    std::string m_szTitle;
    CBaseInstall *m_pInstaller;
    
    virtual CBaseLuaInputField *CreateInputField(const char *label, const char *desc, const char *val,
            int max, const char *type) = 0;
    virtual CBaseLuaCheckbox *CreateCheckbox(const char *desc, const std::vector<std::string> &l) = 0;
    virtual CBaseLuaRadioButton *CreateRadioButton(const char *desc, const std::vector<std::string> &l) = 0;
    virtual CBaseLuaDirSelector *CreateDirSelector(const char *desc, const char *val) = 0;
    virtual CBaseLuaCFGMenu *CreateCFGMenu(const char *desc) = 0;
    virtual void CoreUpdateLanguage(void) { }
    
    bool CallLuaBoolFunc(const char *func, bool def);
    
protected:
    const std::string &GetTitle(void) { return m_szTitle; }
    CBaseInstall *GetInstall(void) { return m_pInstaller; }

public:
    CBaseScreen(const std::string &title) : m_szTitle(title) { }
    virtual ~CBaseScreen(void) { }
    
    void UpdateLanguage(void) { CoreUpdateLanguage(); }
    bool CanActivate(void) { return CallLuaBoolFunc("canactivate", true); }
    void Activate(void);
    bool Back(void) { return CallLuaBoolFunc("back", true); }
    bool Next(void) { return CallLuaBoolFunc("next", true); }
    
    static int LuaAddWidget(lua_State *L);
    static int LuaAddInput(lua_State *L);
    static int LuaAddCheckbox(lua_State *L);
    static int LuaAddRadioButton(lua_State *L);
    static int LuaAddDirSelector(lua_State *L);
    static int LuaAddCFGMenu(lua_State *L);
    
    static void LuaRegister(void);
};

#endif
