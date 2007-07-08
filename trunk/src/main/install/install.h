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
    typedef std::map<char *, arch_size_entry_s> TArchType;
    
    unsigned long m_ulTotalArchSize;
    float m_fExtrPercent;
    TArchType m_ArchList;
    char *m_szCurArchFName;
    bool m_bAlwaysRoot; // If we need root access during whole installation
    short m_sInstallSteps; // Count of things we got to do for installing(extracting files, running commands etc)
    short m_sCurrentStep;
    float m_fInstallProgress;
    int m_iUpdateStatLuaFunc, m_iUpdateProgLuaFunc, m_iUpdateOutputLuaFunc;
     
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
    
protected:
    bool m_bInstalling;

    virtual void InstallThink(void) { }; // Called during installation, so that frontends don't have to block for example
    virtual void InitLua(void);
    
public:
    install_info_s m_InstallInfo;
    std::string m_szBinDir;

    CBaseInstall(void);
    virtual ~CBaseInstall(void) { };

    virtual void Init(int argc, char **argv);

    const char *GetWelcomeFName(void) { return CreateText("%s/config/welcome", m_szOwnDir.c_str()); };
    const char *GetLangWelcomeFName(void)
    { return CreateText("%s/config/lang/%s/welcome", m_szOwnDir.c_str(), m_szCurLang.c_str()); };
    const char *GetLicenseFName(void) { return CreateText("%s/config/license", m_szOwnDir.c_str()); };
    const char *GetLangLicenseFName(void)
    { return CreateText("%s/config/lang/%s/license", m_szOwnDir.c_str(), m_szCurLang.c_str()); };
    const char *GetFinishFName(void) { return CreateText("%s/config/finish", m_szOwnDir.c_str()); };
    const char *GetLangFinishFName(void)
    { return CreateText("%s/config/lang/%s/finish", m_szOwnDir.c_str(), m_szCurLang.c_str()); };
    const char *GetIntroPicFName(void)
    { return CreateText("%s/%s", m_szOwnDir.c_str(), m_InstallInfo.intropicname.c_str()); };

    void UpdateExtrStatus(const char *s);
    
    void SetDestDir(const std::string &dir) { SetDestDir(dir.c_str()); }
    void SetDestDir(const char *dir);
    std::string GetDestDir(void);
    bool VerifyDestDir();

    virtual CBaseScreen *CreateScreen(const std::string &title) = 0;
    virtual void AddScreen(int luaindex) = 0;
    
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
};

inline CBaseInstall *GetFromClosure(lua_State *L)
{ return reinterpret_cast<CBaseInstall *>(lua_touserdata(L, lua_upvalueindex(1))); }
