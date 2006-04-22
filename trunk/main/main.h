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

#ifndef MAIN_H
#define MAIN_H

#include "libsu.h"

#include <stdarg.h>
#include <string>
#include <list>
#include <vector>
#include <map>

enum EArchiveType { ARCH_GZIP, ARCH_BZIP2 };
enum ENeedRoot { NO_ROOT, NEED_ROOT, DEPENDED_ROOT };
enum EParamType { PTYPE_STRING, PTYPE_DIR, PTYPE_LIST, PTYPE_BOOL };
enum EDestDirType { DEST_TEMP, DEST_SELECT, DEST_DEFAULT };

struct param_entry_s
{
    std::string parameter, defaultval, description, varname;
    std::list<std::string> options;
    std::string value;
    EParamType param_type;
    param_entry_s(void) : param_type(PTYPE_BOOL) { };
};

struct command_entry_s
{
    ENeedRoot need_root;
    std::string command, description, path;
    std::map<std::string, param_entry_s *> parameter_entries;
    std::string dep_param;
    bool exit_on_failure;
    command_entry_s(void) : need_root(NO_ROOT), path("/bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin:."),
                            exit_on_failure(true) { };
};

struct install_info_s
{
    std::string version, program_name, description, url, intropicname;
    EArchiveType archive_type;
    EDestDirType dest_dir_type;
    std::list<command_entry_s *> command_entries;
    install_info_s(void) : archive_type(ARCH_GZIP), dest_dir_type(DEST_SELECT) { };
};

struct arch_size_entry_s
{
    unsigned int totalsize;
    std::map<std::string, unsigned int> filesizes;
    arch_size_entry_s(void) : totalsize(1) { };
};

struct app_entry_s
{
    std::string name, version, description, url;
    std::map<std::string, std::string> FileSums;
    app_entry_s(void) : name("-"), version("-"), description("-"), url("-") { };
};

extern std::list<char *> StringList; // List of all strings created by CreateText

char *CreateText(const char *s, ...);
inline char *MakeCString(const std::string &s) { return CreateText(s.c_str()); };
void FreeStrings(void);
bool FileExists(const char *file);
inline bool FileExists(const std::string &file) { return FileExists(file.c_str()); };
bool WriteAccess(const char *file);
inline bool WriteAccess(const std::string &file) { return WriteAccess(file.c_str()); };
bool ReadAccess(const char *file);
inline bool ReadAccess(const std::string &file) { return ReadAccess(file.c_str()); };
void CleanPasswdString(char *str);
std::string &EatWhite(std::string &str, bool skipnewlines=false);
void MakeStringList(const std::string &str, std::vector<char *> &strlist);
void MakeStringList(const char *str, std::vector<char *> &strlist);
void GetTextFromBlock(std::ifstream &file, std::string &text);
std::string GetFirstValidDir(const std::string &dir);
std::string GetMD5(const std::string &file);

// These functions should be defined for each frontend
void EndProg(bool err=false);

class CMain;

class CRegister
{
    CMain *m_pOwner;
    char *m_szConfDir;
    const std::string m_szRegVer;
    LIBSU::CLibSU m_SUHandler;
    
    void WriteRegEntry(const char *entry, const std::string &field, std::ofstream &file);
    std::string ReadRegField(std::ifstream &file);
    const char *GetAppRegDir(void);
    const char *GetConfFile(const char *progname);
    const char *GetSumListFile(const char *progname);
    app_entry_s *GetAppEntry(const char *progname);
    void WriteSums(const char *filename, std::ofstream &outfile, const std::string *var);
    
public:
    typedef void (*TUpFunc)(int, const std::string &, void *);
    typedef char *(*TPasFunc)(void *);
    enum EUninstRet { UNINST_SUCCESS, UNINST_WRONGPASS, UNINST_NULLPASS, UNINST_SUERR };

    CRegister(CMain *owner) : m_pOwner(owner), m_szConfDir(NULL), m_szRegVer("1.0") { };

    bool IsInstalled(bool checkver);
    void RemoveFromRegister(app_entry_s *pApp);
    void RegisterInstall(void);
    EUninstRet Uninstall(app_entry_s *pApp, bool checksum, TUpFunc UpFunc, TPasFunc PasFunc, void *pData);
    void GetRegisterEntries(std::vector<app_entry_s *> *AppVec);
    void CalcSums(const char *dir);
    bool CheckSums(const char *progname);
};

class CMain
{
protected:
    const std::string m_szRegVer;
    char *m_szAppConfDir;
    LIBSU::CLibSU m_SUHandler;
    char *m_szPassword;

    void SetUpSU(const char *msg);
    bool ReadLang(void);
    
    virtual char *GetPassword(const char *title) = 0;
    virtual void MsgBox(const char *str, ...) = 0;
    virtual bool YesNoBox(const char *str, ...) = 0;
    virtual int ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...) = 0;
    virtual void Warn(const char *str, ...) = 0;
    virtual bool ReadConfig(void) = 0;
    
    // App register stuff
    std::string ReadRegField(std::ifstream &file);
    app_entry_s *GetAppRegEntry(const char *progname);
    const char *GetAppRegDir(void);
    const char *GetRegConfFile(const char *progname);
    const char *GetSumListFile(const char *progname);
    
public:
    std::string m_szCurLang;
    std::list<std::string> m_Languages;
    std::map<std::string, char *> m_Translations;
    
    CMain(void) : m_szRegVer("1.0"), m_szAppConfDir(NULL), m_szPassword(NULL) { };
    virtual ~CMain(void);
    
    void ThrowError(bool dialog, const char *error, ...);
    
    virtual bool Init(int argc, char *argv[]);
    virtual void UpdateLanguage(void) { ReadLang(); };
    
    std::string GetTranslation(std::string &s);
    char *GetTranslation(char *s);
    inline char *GetTranslation(const char *s) { return GetTranslation(const_cast<char *>(s)); };
};
    
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
    
protected:
    
    void VerifyDestDir(void);
    
    virtual void ChangeStatusText(const char *str, int curstep, int maxsteps) = 0;
    virtual void AddInstOutput(const std::string &str) = 0;
    virtual void SetProgress(int percent) = 0;
    
    virtual bool ReadConfig(void);
    
public:
    install_info_s m_InstallInfo;
    std::string m_szOS, m_szCPUArch, m_szOwnDir, m_szDestDir;

    
    CBaseInstall(void) : m_iTotalArchSize(1), m_fExtrPercent(0.0f), m_szCurArchFName(NULL),
                         m_bAlwaysRoot(false), m_sInstallSteps(0), m_sCurrentStep(0), m_fInstallProgress(0.0f) { };
    virtual ~CBaseInstall(void);
    
    virtual bool Init(int argc, char *argv[]);
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
    
    static void ExtrSUOutFunc(const char *s, void *p) { ((CBaseInstall *)p)->UpdateStatus(s); };
};

class CBaseAppManager: virtual public CMain
{
    const char *GetSumListFile(const char *progname);
    app_entry_s *GetAppEntry(const char *progname);
    
protected:
    virtual bool ReadConfig(void) { return true; }; // UNDONE
    
    void RemoveFromRegister(app_entry_s *pApp);
    void Uninstall(app_entry_s *pApp, bool checksum);
    void GetRegisterEntries(std::vector<app_entry_s *> *AppVec);
    bool CheckSums(const char *progname);
    
    virtual void AddUninstOutput(const std::string &str) = 0;
    virtual void SetProgress(int percent) = 0;
    
public:
    
    ~CBaseAppManager(void) { };
};
        
//#define RELEASE /* Enable on a release build */

#ifdef RELEASE
inline void debugline(const char *, ...) { };
#else
//#define debugline printf
void debugline(const char *t, ...); // Defined in frontend code
#endif

#endif 
