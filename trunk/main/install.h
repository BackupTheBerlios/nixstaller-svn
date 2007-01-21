/*
    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.

    Nixstaller is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Nixstaller is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nixstaller; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
    conditions of the GNU General Public License cover the whole combination.

    In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
    software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
    DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
    such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
    you include the source code of that other code when and as the GNU GPL requires distribution of source code.

    Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
    versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
    version without this exception; this exception also makes it possible to release a modified version which carries forward
    this exception.
*/

class CBaseCFGScreen;

class CBaseInstall: virtual public CMain
{
    unsigned long m_ulTotalArchSize;
    float m_fExtrPercent;
    std::map<char *, arch_size_entry_s> m_ArchList;
    std::map<char *, arch_size_entry_s>::iterator m_CurArchIter;
    char *m_szCurArchFName;
    bool m_bAlwaysRoot; // If we need root access during whole installation
    short m_sInstallSteps; // Count of things we got to do for installing(extracting files, running commands etc)
    short m_sCurrentStep;
    float m_fInstallProgress;
     
    void InitArchive(char *archname);
    void ExtractFiles(void);
    void ExecuteCommand(const char *cmd, const char *path, bool required);
    void ExecuteCommandAsRoot(const char *cmd, const char *path, bool required);
    const char *GetDefaultPath(void) const { return "/bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin:."; };
    void VerifyIfInstalling(void);
    void UpdateStatusText(const char *str);
    
    // App register stuff
    void WriteSums(const char *filename, std::ofstream &outfile, const std::string *var);
    void WriteRegEntry(const char *entry, const std::string &field, std::ofstream &file);
    void CalcSums(void);
    void RegisterInstall(void);
    bool IsInstalled(bool checkver);
    
    friend class CBaseCFGScreen;
    friend class CBaseLuaInputField;
    friend class CBaseLuaCheckbox;
    friend class CBaseLuaRadioButton;
    friend class CBaseLuaDirSelector;
    friend class CBaseLuaCFGMenu;
    
protected:
    bool m_bInstalling;

    virtual void ChangeStatusText(const char *str) = 0;
    virtual void AddInstOutput(const std::string &str) = 0;
    virtual void SetProgress(int percent) = 0;
    virtual void InstallThink(void) { }; // Called during installation, so that frontends don't have to block for example
    virtual void InitLua(void);
    
public:
    install_info_s m_InstallInfo;
    std::string m_szBinDir;


    CBaseInstall(void) : m_ulTotalArchSize(1), m_fExtrPercent(0.0f), m_szCurArchFName(NULL),
                         m_bAlwaysRoot(false), m_sInstallSteps(1), m_sCurrentStep(0),
                         m_fInstallProgress(0.0f), m_bInstalling(false) { };
    virtual ~CBaseInstall(void) { };

    virtual void Init(int argc, char **argv);
    virtual void Install(void);

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
    void SetDestDir(const char *dir) { m_LuaVM.RegisterString(dir, "destdir"); };
    const char *GetDestDir(void) { return m_LuaVM.GetStrVar("destdir"); };
    bool VerifyDestDir();

    virtual CBaseCFGScreen *CreateCFGScreen(const char *title) = 0;
    
    static void ExtrSUOutFunc(const char *s, void *p) { ((CBaseInstall *)p)->UpdateExtrStatus(s); };
    static void CMDSUOutFunc(const char *s, void *p) { ((CBaseInstall *)p)->AddInstOutput(std::string(s)); };
    static void SUThinkFunc(void *p) { ((CBaseInstall *)p)->InstallThink(); };
    
    // Functions for lua binding
    static int LuaGetTempDir(lua_State *L);
    static int LuaNewCFGScreen(lua_State *L);
    static int LuaExtractFiles(lua_State *L);
    static int LuaExecuteCMD(lua_State *L);
    static int LuaExecuteCMDAsRoot(lua_State *L);
    static int LuaAskRootPW(lua_State *L);
    static int LuaSetStatusMSG(lua_State *L);
    static int LuaSetStepCount(lua_State *L);
    static int LuaPrintInstOutput(lua_State *L);
};

class CBaseLuaInputField
{
    std::string m_szType;
    
protected:
    const std::string &GetType(void) { return m_szType; };
    
public:
    CBaseLuaInputField(const char *t);
    virtual ~CBaseLuaInputField(void) { };
    virtual const char *GetValue(void) = 0;
    virtual void SetSpacing(int percent) = 0;
    
    int GetDefaultSpacing(void) const { return 25; };
    
    static int LuaGet(lua_State *L);
    static int LuaSetSpace(lua_State *L);
};

class CBaseLuaCheckbox
{
public:
    virtual ~CBaseLuaCheckbox(void) { };
    virtual bool Enabled(int n) = 0;
    virtual void Enable(int n, bool b) = 0;
    
    static int LuaGet(lua_State *L);
    static int LuaSet(lua_State *L);
};

class CBaseLuaRadioButton
{
public:
    virtual ~CBaseLuaRadioButton(void) { };
    virtual int EnabledButton(void) = 0;
    virtual void Enable(int n) = 0;
    
    static int LuaGet(lua_State *L);
    static int LuaSet(lua_State *L);
};

class CBaseLuaDirSelector
{
public:
    virtual ~CBaseLuaDirSelector(void) { };
    virtual const char *GetDir(void) = 0;
    virtual void SetDir(const char *dir) = 0;

    static int LuaGet(lua_State *L);
    static int LuaSet(lua_State *L);
};

class CBaseLuaCFGMenu
{
protected:
    enum EVarType { TYPE_DIR, TYPE_STRING, TYPE_LIST, TYPE_BOOL };

    typedef std::vector<std::string> TOptionsType;

    struct entry_s
    {
        std::string val, def, desc;
        TOptionsType options;
        EVarType type;
        entry_s(const char *v, const char *d, EVarType t) : val(v), def(v), desc(d), type(t) { };
    };
    
    typedef std::map<std::string, entry_s *> TVarType;

    TVarType m_Variabeles;
    
    const char *BoolToStr(bool b) { return (b) ? "Enable" : "Disable"; };
    bool StrToBool(const char *s) { if (s && *s && !strcmp(s, "Enable")) return true; else return false; };
    bool StrToBool(const std::string &s) { if (!s.empty() && (s == "Enable")) return true; else return false; };

public:
    virtual ~CBaseLuaCFGMenu(void);
    virtual void AddVar(const char *name, const char *desc, const char *val, EVarType type, TOptionsType *l=NULL);
    
    static int LuaAddDir(lua_State *L);
    static int LuaAddString(lua_State *L);
    static int LuaAddList(lua_State *L);
    static int LuaAddBool(lua_State *L);
    static int LuaGet(lua_State *L);
};

class CBaseCFGScreen
{
public:
    virtual ~CBaseCFGScreen(void) { };
    
    virtual CBaseLuaInputField *CreateInputField(const char *label, const char *desc, const char *val,
                                                 int max, const char *type) = 0;
    virtual CBaseLuaCheckbox *CreateCheckbox(const char *desc, const std::vector<std::string> &l) = 0;
    virtual CBaseLuaRadioButton *CreateRadioButton(const char *desc, const std::vector<std::string> &l) = 0;
    virtual CBaseLuaDirSelector *CreateDirSelector(const char *desc, const char *val) = 0;
    virtual CBaseLuaCFGMenu *CreateCFGMenu(const char *desc) = 0;
    
    static int LuaAddInput(lua_State *L);
    static int LuaAddCheckbox(lua_State *L);
    static int LuaAddRadioButton(lua_State *L);
    static int LuaAddDirSelector(lua_State *L);
    static int LuaAddCFGMenu(lua_State *L);
};
