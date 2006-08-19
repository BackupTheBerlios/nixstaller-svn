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
    int m_iTotalArchSize;
    float m_fExtrPercent;
    std::map<char *, arch_size_entry_s> m_ArchList;
    std::map<char *, arch_size_entry_s>::iterator m_CurArchIter;
    char *m_szCurArchFName;
    bool m_bAlwaysRoot; // If we need root access during whole installation
    short m_sInstallSteps; // Count of things we got to do for installing(extracting files, running commands etc)
    short m_sCurrentStep;
    float m_fInstallProgress;
     
    void SetNextStep(void);
    void InitArchive(char *archname);
    void ExtractFiles(void);
    void ExecuteInstCommands(void);
    
    // App register stuff
    void WriteSums(const char *filename, std::ofstream &outfile, const std::string *var);
    void WriteRegEntry(const char *entry, const std::string &field, std::ofstream &file);
    void CalcSums(void);
    void RegisterInstall(void);
    bool IsInstalled(bool checkver);
    
    friend class CBaseCFGScreen;
    friend class CBaseLuaInputField;
    
protected:
    virtual void ChangeStatusText(const char *str, int curstep, int maxsteps) = 0;
    virtual void AddInstOutput(const std::string &str) = 0;
    virtual void SetProgress(int percent) = 0;

    virtual bool InitLua(void);
    virtual bool ReadConfig(void);

public:
    install_info_s m_InstallInfo;
    std::string m_szDestDir, m_szBinDir;


    CBaseInstall(void) : m_iTotalArchSize(1), m_fExtrPercent(0.0f), m_szCurArchFName(NULL),
                         m_bAlwaysRoot(false), m_sInstallSteps(0), m_sCurrentStep(0), m_fInstallProgress(0.0f) { };
    virtual ~CBaseInstall(void);

    virtual bool Init(int argc, char **argv);
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

    param_entry_s *GetParamByName(std::string str);
    param_entry_s *GetParamByVar(std::string str);
    const char *GetParamDefault(param_entry_s *pParam);
    const char *GetParamValue(param_entry_s *pParam);
    std::string GetParameters(command_entry_s *pCommandEntry);

    void UpdateStatus(const char *s);

    virtual CBaseCFGScreen *CreateCFGScreen(const char *title) = 0;
    
    static void ExtrSUOutFunc(const char *s, void *p) { ((CBaseInstall *)p)->UpdateStatus(s); };

    // Functions for lua binding
    static int LuaNewCFGScreen(lua_State *L);
};

class CBaseLuaInputField
{
public:
    virtual ~CBaseLuaInputField(void) { };
    virtual const char *GetValue(void) = 0;
    
    static int LuaGet(lua_State *L);
};

class CBaseCFGScreen
{
public:
    virtual ~CBaseCFGScreen(void) { };
    
    virtual CBaseLuaInputField *CreateInputField(const char *label, const char *desc, const char *val, int max) = 0;
    
    static int LuaAddInput(lua_State *L);
};
