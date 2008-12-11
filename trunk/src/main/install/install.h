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

#ifndef INSTALL_H
#define INSTALL_H

class CBaseScreen;
class CBaseLuaProgressDialog;
class CBaseLuaDepScreen;

class CBaseInstall: virtual public CMain
{
protected:
    typedef std::vector<CBaseScreen *> TScreenList;
    
private:
    long m_lUITimer, m_lRunTimer;
    TScreenList m_ScreenList;
    CBaseScreen *m_pCurScreen;
    int m_iAskQuitLuaFunc;
     
    void UpdateUI(void);
    void AddScreen(CBaseScreen *screen);
    int ExecuteCommand(const char *cmd, bool required, const char *path, int luaout);
    int ExecuteCommandAsRoot(const char *cmd, bool required, const char *path, int luaout);
    const char *GetDefaultPath(void) const { return "/bin:/sbin:/usr/sbin:/usr/bin:/usr/local/bin:/usr/X11R6/bin:."; };
    void VerifyIfInstalling(void);
    
    // App register stuff
    void WriteSums(const char *filename, std::ofstream &outfile, const std::string *var);
    void WriteRegEntry(const char *entry, const std::string &field, std::ofstream &file);
    void CalcSums(void);
    void RegisterInstall(void);
    bool IsInstalled(bool checkver);
    
    virtual void CoreUpdateUI(void) = 0; // Called during installation, lua execution etc
    virtual CBaseScreen *CreateScreen(const std::string &title) = 0;
    virtual void CoreAddScreen(CBaseScreen *screen) = 0;
    virtual CBaseLuaProgressDialog *CoreCreateProgDialog(int r) = 0;
    virtual CBaseLuaDepScreen *CoreCreateDepScreen(int f) = 0;
    virtual void LockScreen(bool cancel, bool prev, bool next) = 0;
    
protected:
    virtual void InitLua(void);
    virtual void CoreUpdateLanguage(void);

    // This function is not called by the destructor, because in some frontends a lua screen is part of the gui and will
    // be automaticly deleted
    void DeleteScreens(void);
    TScreenList &GetScreenList(void) { return m_ScreenList; }
    void ActivateScreen(CBaseScreen *screen);
    void Run(void);
    
public:
    std::string m_szBinDir;

    CBaseInstall(void);
    virtual ~CBaseInstall(void) { };

    virtual void Init(int argc, char **argv);
    
    void SetDestDir(const std::string &dir) { SetDestDir(dir.c_str()); }
    void SetDestDir(const char *dir);
    std::string GetDestDir(void);
    bool VerifyDestDir(void);
    bool AskQuit(void);

    static void CMDSUOutFunc(const char *s, void *p);
    static void SUThinkFunc(void *p) { ((CBaseInstall *)p)->UpdateUI(); };
    
    static void LuaHook(lua_State *L, lua_Debug *ar);
    
    // Functions for lua binding
    static int LuaNewScreen(lua_State *L);
    static int LuaAddScreen(lua_State *L);
    static int LuaNewProgressDialog(lua_State *L);
    static int LuaShowDepScreen(lua_State *L);
    static int LuaGetTempDir(lua_State *L);
    static int LuaGetPkgDir(lua_State *L);
    static int LuaGetMacAppPath(lua_State *L);
    static int LuaExecuteCMD(lua_State *L);
    static int LuaExecuteCMDAsRoot(lua_State *L);
    static int LuaAskRootPW(lua_State *L);
    static int LuaLockScreen(lua_State *L);
    static int LuaVerifyDestDir(lua_State *L);
    static int LuaExtraFilesPath(lua_State *L);
    static int LuaGetLang(lua_State *L);
    static int LuaSetLang(lua_State *L);
    static int LuaUpdateUI(lua_State *L);
    static int LuaSetAskQuit(lua_State *L);
};

// Utils
CBaseInstall *GetFromClosure(lua_State *L);

#endif
