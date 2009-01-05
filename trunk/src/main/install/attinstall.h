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

#ifndef ATT_INSTALL_H
#define ATT_INSTALL_H

#include "main/install/install.h"

class CBaseScreen;
class CBaseLuaProgressDialog;
class CBaseLuaDepScreen;

class CBaseAttInstall: virtual public CBaseInstall
{
protected:
    typedef std::vector<CBaseScreen *> TScreenList;
    
private:
    LIBSU::CLibSU m_SUHandler;
    char *m_szPassword;
    TScreenList m_ScreenList;
    CBaseScreen *m_pCurScreen;
    int m_iAskQuitLuaFunc;
     
    bool GetSUPasswd(const char *msg, bool mandatory);
    void AddScreen(CBaseScreen *screen);
    int ExecuteCommandAsRoot(const char *cmd, bool required, const char *path, int luaout);
    
    virtual char *GetPassword(const char *title) = 0;
    virtual void MsgBox(const char *str, ...) = 0;
    virtual bool YesNoBox(const char *str, ...) = 0;
    virtual int ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...) = 0;
    virtual void WarnBox(const char *str, ...) = 0;
    virtual CBaseScreen *CreateScreen(const std::string &title) = 0;
    virtual void CoreAddScreen(CBaseScreen *screen) = 0;
    virtual CBaseLuaProgressDialog *CoreCreateProgDialog(int r) = 0;
    virtual CBaseLuaDepScreen *CoreCreateDepScreen(int f) = 0;
    virtual void LockScreen(bool cancel, bool prev, bool next) = 0;
    
protected:
    virtual void InitLua(void);
    virtual void CoreUpdateLanguage(void);
    virtual void CoreUpdate(void);

    // This function is not called by the destructor, because in some frontends a lua screen is part of the gui and will
    // be automaticly deleted
    void DeleteScreens(void);
    TScreenList &GetScreenList(void) { return m_ScreenList; }
    void ActivateScreen(CBaseScreen *screen);
    
public:
    CBaseAttInstall(void);
    virtual ~CBaseAttInstall(void) { CleanPasswdString(m_szPassword); };

    virtual void Init(int argc, char **argv);
    
    std::string GetDestDir(void);
    bool VerifyDestDir(void);
    bool AskQuit(void);

    static void CMDSUOutFunc(const char *s, void *p);
    static void SUThinkFunc(void *p) { ((CBaseAttInstall *)p)->Update(); };
    
    static void LuaHook(lua_State *L, lua_Debug *ar);
    
    // Functions for lua binding
    static int LuaMSGBox(lua_State *L);
    static int LuaYesNoBox(lua_State *L);
    static int LuaChoiceBox(lua_State *L);
    static int LuaWarnBox(lua_State *L);
    static int LuaNewScreen(lua_State *L);
    static int LuaAddScreen(lua_State *L);
    static int LuaNewProgressDialog(lua_State *L);
    static int LuaShowDepScreen(lua_State *L);
    static int LuaExecuteCMDAsRoot(lua_State *L);
    static int LuaAskRootPW(lua_State *L);
    static int LuaLockScreen(lua_State *L);
    static int LuaVerifyDestDir(lua_State *L);
    static int LuaSetAskQuit(lua_State *L);
};

#endif
