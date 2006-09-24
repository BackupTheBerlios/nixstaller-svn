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
#include "lua.hpp"

#include <stdarg.h>
#include <string>
#include <list>
#include <vector>
#include <map>

enum EArchiveType { ARCH_GZIP, ARCH_BZIP2, ARCH_LZMA };
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

void Intro(void);
char *CreateText(const char *s, ...);
inline char *MakeCString(const std::string &s) { return CreateText(s.c_str()); };
char *CreateTmpText(const char *s, ...);
void FreeStrings(void);
bool FileExists(const char *file);
inline bool FileExists(const std::string &file) { return FileExists(file.c_str()); };
bool WriteAccess(const char *file);
inline bool WriteAccess(const std::string &file) { return WriteAccess(file.c_str()); };
bool ReadAccess(const char *file);
inline bool ReadAccess(const std::string &file) { return ReadAccess(file.c_str()); };
bool IsDir(const char *file);
void *guaranteed_memset(void *v,int c,size_t n);
void CleanPasswdString(char *str);
std::string &EatWhite(std::string &str, bool skipnewlines=false);
void MakeStringList(const std::string &str, std::vector<char *> &strlist);
void MakeStringList(const char *str, std::vector<char *> &strlist);
void GetTextFromBlock(std::ifstream &file, std::string &text);
std::string GetFirstValidDir(const std::string &dir);
std::string GetMD5(const std::string &file);
mode_t StrToMode(const char *str);
unsigned StrFindInRange(const std::string &src, const std::string &findstr, unsigned pos, unsigned n);
unsigned StrFindInRange(const std::string &src, const char *findstr, unsigned pos, unsigned n);
std::string GetTranslation(std::string &s);
char *GetTranslation(char *s);
inline char *GetTranslation(const char *s) { return GetTranslation(const_cast<char *>(s)); };

inline int Min(int n1, int n2) { return (n1 < n2) ? n1 : n2; };
inline int Max(int n1, int n2) { return (n1 > n2) ? n1 : n2; };

// These functions should be defined for each frontend
void EndProg(bool err=false);

#ifndef HAVE_VASPRINTF
extern "C" int vasprintf (char **result, const char *format, va_list args);
#endif

class CLuaVM
{
    lua_State *m_pLuaState;
    int m_iPushedArgs;
    
    void StackDump(const char *msg);
    void GetGlobal(const char *var, const char *tab);
    
public:
    CLuaVM(void) : m_iPushedArgs(0) { };
    ~CLuaVM(void) { lua_close(m_pLuaState); };

    bool Init(void);
    bool LoadFile(const char *name);
    
    void RegisterFunction(lua_CFunction f, const char *name, const char *tab=NULL, void *data=NULL);
    void RegisterNumber(lua_Number n, const char *name, const char *tab=NULL);
    void RegisterString(const char *s, const char *name, const char *tab=NULL);
    template <typename C> void RegisterUData(C val, const char *type, const char *name, const char *tab=NULL)
    {
        if (!tab)
        {
            CreateClass<C>(val, type);
            lua_setglobal(m_pLuaState, name);
        }
        else
        {
            lua_getglobal(m_pLuaState, tab);
        
            if (lua_isnil(m_pLuaState, -1))
            {
                lua_pop(m_pLuaState, 1);
                lua_newtable(m_pLuaState);
            }
        
            CreateClass<C>(val, type);
            lua_setfield(m_pLuaState, -2, name);
        
            lua_setglobal(m_pLuaState, tab);
        }
    }
    
    bool InitCall(const char *func, const char *tab=NULL);
    void PushArg(lua_Number n);
    void PushArg(const char *s);
    void DoCall(void);
    
    bool GetNumVar(lua_Number *out, const char *var, const char *tab=NULL);
    bool GetNumVar(lua_Integer *out, const char *var, const char *tab=NULL);
    bool GetStrVar(std::string *out, const char *var, const char *tab=NULL);
    
    unsigned OpenArray(const char *var, const char *tab=NULL);
    bool GetArrayNum(unsigned &index, lua_Number *out);
    bool GetArrayNum(unsigned &index, lua_Integer *out);
    bool GetArrayStr(unsigned &index, std::string *out);
    bool GetArrayStr(unsigned &index, char *out);
    template <typename C> bool GetArrayClass(unsigned &index, C *out, const char *type)
    {
        lua_rawgeti(m_pLuaState, -1, index);

        C val = ToClass<C>(type, -1);
        lua_pop(m_pLuaState, 1);
    
        if (val)
        {
            *out = val;
            return true;
        }
        
        return false;
    }
    void CloseArray(void);
    
    void InitClass(const char *name, lua_CFunction gc=NULL, void *gcdata=NULL);
    template <typename C> void CreateClass(C val, const char *type)
    {
        C *ud = (C *)lua_newuserdata(m_pLuaState, sizeof(C));
        *ud = val;
    
        luaL_getmetatable(m_pLuaState, type);
        lua_setmetatable(m_pLuaState, -2);
    }
    void RegisterClassFunc(const char *type, lua_CFunction f, const char *name, void *data=NULL);
    template <typename C> C CheckClass(const char *type, int index)
    {
        luaL_checktype(m_pLuaState, index, LUA_TUSERDATA);
        C *val = (C *)luaL_checkudata(m_pLuaState, index, type);
        if (!val)
            luaL_typerror(m_pLuaState, index, type);
        else if (!(*val))
            luaL_error(m_pLuaState, "Got a NULL value for class %s", type);
    
        return *val;
    }
    template <typename C> C ToClass(const char *type, int index)
    {
        C *val = (C *)lua_touserdata(m_pLuaState, index), *ret = NULL;
        if (val)
        {
            if (lua_getmetatable(m_pLuaState, index))
            {
                lua_getfield(m_pLuaState, LUA_REGISTRYINDEX, type);
                if (lua_rawequal(m_pLuaState, -1, -2))
                    ret = val;
                lua_pop(m_pLuaState, 1);
            }
            lua_pop(m_pLuaState, 1);
        }
        return (ret) ? *ret : NULL;
    }
            
    void SetArrayNum(lua_Number n, const char *tab, int index);
    void SetArrayStr(const char *s, const char *tab, int index);

    void *GetClosure(void);
    bool GetArgNum(lua_Number *out);
    bool GetArgNum(lua_Integer *out);
    bool GetArgStr(std::string *out);
    bool GetArgStr(char *out);
};

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
    void CreateInstall(int argc, char **argv);
    
protected:
    CLuaVM m_LuaVM;
    const std::string m_szRegVer;
    std::string m_szOS, m_szCPUArch, m_szOwnDir;
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
    
    virtual bool InitLua(void);
    
public:
    std::string m_szCurLang;
    std::list<std::string> m_Languages;
    
    CMain(void) : m_szRegVer("1.0"), m_szAppConfDir(NULL), m_szPassword(NULL) { };
    virtual ~CMain(void);
    
    void ThrowError(bool dialog, const char *error, ...);
    
    virtual bool Init(int argc, char **argv);
    virtual void UpdateLanguage(void) { ReadLang(); };
    
    // Functions for lua binding
    static int LuaInitDirIter(lua_State *L);
    static int LuaDirIter(lua_State *L);
    static int LuaDirIterGC(lua_State *L);
    static int LuaFileExists(lua_State *L);
    static int LuaWritePerm(lua_State *L);
    static int LuaReadPerm(lua_State *L);
    static int LuaIsDir(lua_State *L);
    static int LuaMKDir(lua_State *L);
    static int LuaMKDirRec(lua_State *L);
    static int LuaCPFile(lua_State *L);
    static int LuaCHMod(lua_State *L);
    static int LuaGetCWD(lua_State *L);
    static int LuaCHDir(lua_State *L);
    static int LuaGetFileSize(lua_State *L);
    static int LuaMSGBox(lua_State *L);
};
    
#include "install.h"

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
    
    virtual ~CBaseAppManager(void) { };
};

//#define RELEASE /* Enable on a release build */

#ifdef RELEASE
inline void debugline(const char *, ...) { };
#else
//#define debugline printf
void debugline(const char *t, ...); // Defined in frontend code
#endif

#endif 
