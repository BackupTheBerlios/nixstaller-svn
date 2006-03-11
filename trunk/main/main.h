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
    std::string os, cpuarch, version, program_name, intropicname, own_dir, dest_dir, cur_lang;
    std::list<std::string> languages;
    std::map<std::string, char *> translations;
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

extern install_info_s InstallInfo;
extern std::list<char *> StringList; // List of all strings created by CreateText

bool MainInit(int argc, char *argv[]);
void MainEnd(void);
bool ReadConfig(void);
int ArchSize(const char *archname);
void GetArchiveInfo(const char *archname, std::map<std::string, unsigned int> &archfilesizes, unsigned int &totalsize);
float ExtractArchive(std::string &curfile);
bool ReadLang(void);
std::string GetParameters(command_entry_s *pCommandEntry);
std::string GetTranslation(std::string &s);
char *GetTranslation(char *s);
inline char *GetTranslation(const char *s) { return GetTranslation(const_cast<char *>(s)); };
char *CreateText(const char *s, ...);
inline char *MakeCString(const std::string &s) { return CreateText(s.c_str()); };
void FreeStrings(void);
param_entry_s *GetParamByName(std::string str);
param_entry_s *GetParamByVar(std::string str);
const char *GetParamDefault(param_entry_s *pParam);
const char *GetParamValue(param_entry_s *pParam);
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
void throwerror(bool dialog, const char *error, ...);
void EndProg(bool err=false);

class CExtractAsRootFunctor
{
    int m_iTotalSize;
    float m_fPercent;
    std::map<char *, arch_size_entry_s> m_ArchList;
    std::map<char *, arch_size_entry_s>::iterator m_CurArchIter;
    char *m_szCurArchFName;
    LIBSU::CLibSU m_SUHandler;
    
    typedef void (*TUpProgress)(int percent, void *p);
    typedef void (*TUpText)(const std::string &str, void *p);

    TUpProgress m_UpProgFunc;
    TUpText m_UpTextFunc;
    void *m_pFuncData[2];
    
public:
    bool operator ()(char *passwd);
    void Update(const char *s);
    void SetUpdateProgFunc(TUpProgress UpProgFunc, void *data) { m_UpProgFunc = UpProgFunc; m_pFuncData[0] = data; };
    void SetUpdateTextFunc(TUpText UpTextFunc, void *data) { m_UpTextFunc = UpTextFunc; m_pFuncData[1] = data; };
    static void SUOutFunc(const char *s, void *p) { ((CExtractAsRootFunctor *)p)->Update(s); };
};

class CRegister
{
    char *m_szConfDir;
    
    const char *GetAppRegDir(void);
    const char *GetConfFile(const char *progname);
    const char *GetSumListFile(const char *progname);
    app_entry_s *GetAppEntry(const char *progname);
    void WriteSums(const char *filename, std::ofstream &outfile, const std::string *var);
    
public:
    CRegister(void) : m_szConfDir(NULL) { };
    
    bool IsInstalled(bool checkver);
    void RemoveFromRegister(const char *progname);
    void RegisterInstall(void);
    void Uninstall(const char *progname, bool checksum);
    void GetRegisterEntries(std::vector<app_entry_s *> *AppVec);
    void CalcSums(void);
    bool CheckSums(const char *progname);
};

extern CRegister Register;

//#define RELEASE /* Enable on a release build */

#ifdef RELEASE
inline void debugline(const char *, ...) { };
#else
//#define debugline printf
inline void debugline(const char *t, ...)
{ static char txt[1024]; va_list v; va_start(v, t); vsprintf(txt, t, v); va_end(v); printf("DEBUG: %s", txt); };
#endif

#endif 
