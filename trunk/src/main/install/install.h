/*
    Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)

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

class CBaseScreen;

class CBaseInstall: virtual public CMain
{
protected:
    typedef std::vector<CBaseScreen *> TScreenList;
    
private:
    typedef std::map<char *, arch_size_entry_s> TArchType;
    
    TScreenList m_ScreenList;
    unsigned long m_ulTotalArchSize;
    float m_fExtrPercent;
    TArchType m_ArchList;
    char *m_szCurArchFName;
    bool m_bAlwaysRoot; // If we need root access during whole installation
    short m_sInstallSteps; // Count of things we got to do for installing(extracting files, running commands etc)
    short m_sCurrentStep;
    float m_fInstallProgress;
    int m_iUpdateStatLuaFunc, m_iUpdateProgLuaFunc, m_iUpdateOutputLuaFunc;
    bool m_bInstalling;
     
    void AddScreen(CBaseScreen *screen);
    void Install(int statluafunc, int progluafunc, int outluafunc);
    void InitArchive(char *archname);
    void ExtractFiles(void);
    void ExecuteCommand(const char *cmd, bool required, const char *path);
    void ExecuteCommandAsRoot(const char *cmd, bool required, const char *path);
    const char *GetDefaultPath(void) const { return "/bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin:."; };
    void VerifyIfInstalling(void);
    void UpdateStatusText(const char *str);
    void AddOutput(const std::string &str);
    void SetProgress(int percent);
    
    // App register stuff
    void WriteSums(const char *filename, std::ofstream &outfile, const std::string *var);
    void WriteRegEntry(const char *entry, const std::string &field, std::ofstream &file);
    void CalcSums(void);
    void RegisterInstall(void);
    bool IsInstalled(bool checkver);
    
    virtual CBaseScreen *CreateScreen(const std::string &title) = 0;
    virtual void CoreAddScreen(CBaseScreen *screen) = 0;
    virtual void InstallThink(void) { }; // Called during installation, so that frontends don't have to block for example
    virtual void LockScreen(bool cancel, bool prev, bool next) = 0;
    
protected:
    virtual void InitLua(void);

    bool Installing(void) const { return m_bInstalling; }
    // This function is not called by the destructor, because in some frontends a lua screen is part of the gui and will
    // be automaticly deleted
    void DeleteScreens(void);
    TScreenList &GetScreenList(void) { return m_ScreenList; }
    
public:
    install_info_s m_InstallInfo;
    std::string m_szBinDir;

    CBaseInstall(void);
    virtual ~CBaseInstall(void) { };

    virtual void Init(int argc, char **argv);
    virtual void UpdateLanguage(void);

    void UpdateExtrStatus(const char *s);
    
    void SetDestDir(const std::string &dir) { SetDestDir(dir.c_str()); }
    void SetDestDir(const char *dir);
    std::string GetDestDir(void);
    bool VerifyDestDir();

    static void ExtrSUOutFunc(const char *s, void *p) { ((CBaseInstall *)p)->UpdateExtrStatus(s); };
    static void CMDSUOutFunc(const char *s, void *p) { ((CBaseInstall *)p)->AddOutput(std::string(s)); };
    static void SUThinkFunc(void *p) { ((CBaseInstall *)p)->InstallThink(); };
    
    // Functions for lua binding
    static int LuaNewScreen(lua_State *L);
    static int LuaAddScreen(lua_State *L);
    static int LuaGetTempDir(lua_State *L);
    static int LuaExtractFiles(lua_State *L);
    static int LuaExecuteCMD(lua_State *L);
    static int LuaExecuteCMDAsRoot(lua_State *L);
    static int LuaAskRootPW(lua_State *L);
    static int LuaSetStatusMSG(lua_State *L);
    static int LuaSetStepCount(lua_State *L);
    static int LuaPrintInstOutput(lua_State *L);
    static int LuaStartInstall(lua_State *L);
    static int LuaLockScreen(lua_State *L);
    static int LuaVerifyDestDir(lua_State *L);
};

// Utils
CBaseInstall *GetFromClosure(lua_State *L);
