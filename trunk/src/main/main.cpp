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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <libgen.h>

#include "main.h"
#include "elfutils.h"
#include "lua/lua.h"
#include "lua/luaclass.h"
#include "lua/luafunc.h"
#include "lua/luatable.h"

std::list<char *> StringList;
std::map<std::string, char *> Translations;

namespace {
bool g_RunScript;
}

// Besides main(), other functions may wat to call this incase it wants to stop an exception flow (ie GTK frontend)
void Quit(int ret)
{
    if (!g_RunScript)
        StopFrontend();
    
#ifndef RELEASE
    NLua::StackDump("Clean stack?\n");
#endif
    
    FreeStrings();
    exit(ret);
}

// Besides main(), other functions may want to call this incase they wants to stop an exception flow (ie GTK frontend)
void HandleError(void)
{
    try
    {
        throw;
    }
    catch(Exceptions::CExUser &e)
    {
        debugline("User cancel\n");
    }
    catch(Exceptions::CException &e)
    {
        if (g_RunScript)
            fprintf(stderr, GetTranslation(e.what())); // No specific way to complain, just use stderr
        else
            ReportError(GetTranslation(e.what()));
    }
    
    Quit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    unlink("frontendstarted");
    
    setlocale(LC_ALL, "");

    PrintIntro();

    LIBSU::SetRunnerPath(dirname(CreateText(argv[0])) + std::string("/../"));
    
    g_RunScript = ((argc >= 5) && !strcmp(argv[1], "-c")); // Caller (usually geninstall.sh) wants to run a lua script?
    
    try
    {
        if (g_RunScript)
        {
            CLuaRunner lr;
            lr.Init(argc, argv);
        }
        else
        {
            StartFrontend(argc, argv);
        }
    }
    catch(Exceptions::CException &e)
    {
        HandleError();
        // Handle error calls exit()
    }
    
    Quit(EXIT_SUCCESS);
    return 0; // Never reached
}

// -------------------------------------
// Main Class
// -------------------------------------

CMain::~CMain()
{
    if (!Translations.empty())
    {
        for(std::map<std::string, char *>::iterator p=Translations.begin(); p!=Translations.end(); p++)
            delete [] (*p).second;
    }
    
//    FreeStrings();
    closelog();
}

void CMain::Init(int argc, char **argv)
{
    // Get current OS and cpu arch name
    struct utsname inf;
    UName(inf);
    
    m_szOS = inf.sysname;
    std::transform(m_szOS.begin(), m_szOS.end(), m_szOS.begin(), tolower);

    m_szCPUArch = inf.machine;
    if ((m_szCPUArch[0] == 'i') && (m_szCPUArch.compare(2, 2, "86") == 0))
        m_szCPUArch = "x86";
    else if (m_szCPUArch == "i86pc")
        m_szCPUArch = "x86";
    else if (m_szCPUArch == "amd64")
        m_szCPUArch = "x86_64";

    m_szOwnDir = GetCWD();
    
    InitLua();
    
    if (m_Languages.empty())
    {
        char *s = new char[8];
        strcpy(s, "english");
        m_Languages.push_back(s);
    }
    
    if (!NLua::LuaGet(m_szCurLang, "defaultlang", "cfg"))
        m_szCurLang = "english";
    
    if (std::find(m_Languages.begin(), m_Languages.end(), m_szCurLang) == m_Languages.end())
        m_szCurLang = m_Languages.front();
    
    debugline("defaultlang: %s\n", m_szCurLang.c_str());
    UpdateLanguage();
    
    m_SUHandler.SetUser("root");
    m_SUHandler.SetTerminalOutput(false);
}

const char *CMain::GetLogoFName()
{
    std::string ret = "installer.png"; // Default
    NLua::LuaGet(ret, "logo", "cfg");
    return CreateText("%s/%s", m_szOwnDir.c_str(), ret.c_str());
}

const char *CMain::GetAboutFName()
{
    return CreateText("%s/about", m_szOwnDir.c_str());
}

bool CMain::GetSUPasswd(const char *msg, bool mandatory)
{
    if ((!m_szPassword || !m_szPassword[0]) && m_SUHandler.NeedPassword())
    {
        while(true)
        {
            CleanPasswdString(m_szPassword);
            
            m_szPassword = GetPassword(GetTranslation(msg));
            
            // Check if password is invalid
            if (!m_szPassword || !m_szPassword[0])
            {
                if (!mandatory)
                    return false;
                
                if (ChoiceBox(GetTranslation("Root access is required to continue\nAbort installation?"),
                    GetTranslation("No"), GetTranslation("Yes"), NULL))
                    throw Exceptions::CExUser();
            }
            else
            {
                if (m_SUHandler.TestSU(m_szPassword))
                    break;

                // Some error appeared
                if (m_SUHandler.GetError() == LIBSU::CLibSU::SU_ERROR_INCORRECTPASS)
                    WarnBox(GetTranslation("Incorrect password given, please retype."));
                else
                {
                    const char *msg = "Could not use su or sudo to gain root access.";
                    
                    if (mandatory)
                        throw Exceptions::CExSU(msg);
                    else
                    {
                        WarnBox(GetTranslation(msg));
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

bool CMain::ReadLang()
{
    // Clear all translations
    if (!Translations.empty())
    {
        std::map<std::string, char *>::iterator p = Translations.begin();
        for(;p!=Translations.end();p++)
            delete [] (*p).second;
        Translations.clear();
    }
    
    std::ifstream file(CreateText("%s/config/lang/%s/strings", m_szOwnDir.c_str(), m_szCurLang.c_str()));

    if (!file)
    {
        debugline("WARNING: Failed to load language file for %s\n", m_szCurLang.c_str());
        return false;
    }
    
    std::string text, srcmsg;
    bool atsrc = true;
    while (file)
    {
        std::getline(file, text);
        EatWhite(text);

        if (text.empty() || text[0] == '#')
            continue;

        if (text[0] == '[')
            GetTextFromBlock(file, text);
        
        if (atsrc)
            srcmsg = text;
        else
        {
            Translations[srcmsg] = new char[text.length()+1];
            text.copy(Translations[srcmsg], std::string::npos);
            Translations[srcmsg][text.length()] = 0;
        }

        atsrc = !atsrc;
    }

    return true;
}

void CMain::UpdateUI()
{
    long curtime = GetTime();
    
    if (m_lUITimer <= curtime)
    {
        m_lUITimer = curtime + 10;
        CoreUpdateUI();
    }
}

#ifdef WITH_APPMANAGER
std::string CMain::ReadRegField(std::ifstream &file)
{
    std::string line, ret;
    std::string::size_type index = 0;
    
    std::getline(file, line);
    EatWhite(line);
    
    if (line[0] != '\"')
        return "";
    
    line.erase(0, 1); // Remove initial "
    while ((index = line.find("\\\"", index+1)) != std::string::npos)
        line.replace(index, 2, "\"");

    ret = line;
    
    while(line[line.length()-1] != '\"' && file && std::getline(file, line))
    {
        EatWhite(line);
        while ((index = line.find("\\\"")) != std::string::npos)
            line.replace(index, 2, "\"");
        ret += "\n" + line;
    }
    
    ret.erase(ret.length()-1, 1); // Remove trailing "
    return ret;
}

app_entry_s *CMain::GetAppRegEntry(const char *progname)
{
    const char *filename = GetRegConfFile(progname);
    if (!FileExists(filename))
        return NULL;

    app_entry_s *pAppEntry = new app_entry_s;
    pAppEntry->name = progname;
    
    std::ifstream file(filename);
    std::string str, field;
    
    while(file && (file >> str))
    {
        field = ReadRegField(file);
        
        if (str == "version")
            pAppEntry->version = field;
        else if (str == "description")
            pAppEntry->description = field;
        else if (str == "url")
            pAppEntry->url = field;
    }

    file.close();

    filename = GetSumListFile(progname);
    if (FileExists(filename))
    {
        std::string line, sum;

        std::ifstream sumfile(filename);
        while(sumfile)
        {
            if (!(sumfile >> sum) || !std::getline(sumfile, line))
                break;
            pAppEntry->FileSums[EatWhite(line)] = sum;
        }
    }
    
    return pAppEntry;
}

const char *CMain::GetAppRegDir()
{
    if (!m_szAppConfDir)
    {
        const char *home = getenv("HOME");
        if (!home)
            ThrowError(false, "Couldn't find out your home directory!");
        
        m_szAppConfDir = CreateText("%s/.nixstaller", home);
        
        if (mkdir(m_szAppConfDir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
            ThrowError(false, "Could not create nixstaller config directory!(%s)", strerror(errno));
    }
    
    return m_szAppConfDir;
}

const char *CMain::GetRegConfFile(const char *progname)
{
    const char *dir = CreateText("%s/%s", GetAppRegDir(), progname);
    if (mkdir(dir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
        ThrowError(false, "Could not create nixstaller app-config directory!(%s)", strerror(errno));
    return CreateText("%s/config", dir);
}

const char *CMain::GetSumListFile(const char *progname)
{
    const char *dir = CreateText("%s/%s", GetAppRegDir(), progname);
    if (mkdir(dir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
        ThrowError(false, "Could not create nixstaller app-config directory!(%s)", strerror(errno));
    return CreateText("%s/list", dir);
}
#endif

void CMain::InitLua()
{
    lua_atpanic(NLua::LuaState, LuaPanic);
    
    // Register some globals for lua
    NLua::LuaSet("0.4", "version");
    
    NLua::LuaSet(m_szOS, "osname", "os");
    NLua::LuaSet(m_szCPUArch, "arch", "os");
    NLua::LuaSet(m_szOwnDir, "curdir");
    
    NLua::RegisterFunction(LuaTr, "tr");
    NLua::RegisterFunction(LuaInitDirIter, "dir", "io");
    NLua::RegisterFunction(LuaMD5, "md5", "io");
    
    NLua::RegisterFunction(LuaFileExists, "fileexists", "os");
    NLua::RegisterFunction(LuaReadPerm, "readperm", "os");
    NLua::RegisterFunction(LuaWritePerm, "writeperm", "os");
    NLua::RegisterFunction(LuaIsDir, "isdir", "os");
    NLua::RegisterFunction(LuaMKDir, "mkdir", "os");
    NLua::RegisterFunction(LuaMKDirRec, "mkdirrec", "os");
    NLua::RegisterFunction(LuaCPFile, "copy", "os");
    NLua::RegisterFunction(LuaCHMod, "chmod", "os");
    NLua::RegisterFunction(LuaGetCWD, "getcwd", "os");
    NLua::RegisterFunction(LuaCHDir, "chdir", "os");
    NLua::RegisterFunction(LuaGetFileSize, "filesize", "os");
    NLua::RegisterFunction(LuaLog, "log", "os", this);
    NLua::RegisterFunction(LuaSetEnv, "setenv", "os", this);
    NLua::RegisterFunction(LuaExit, "exit", "os"); // Override
    NLua::RegisterFunction(LuaExitStatus, "exitstatus", "os");
    NLua::RegisterFunction(LuaOpenElf, "openelf", "os");

    NLua::RegisterFunction(LuaMSGBox, "msgbox", "gui", this);
    NLua::RegisterFunction(LuaYesNoBox, "yesnobox", "gui", this);
    NLua::RegisterFunction(LuaChoiceBox, "choicebox", "gui", this);
    NLua::RegisterFunction(LuaWarnBox, "warnbox", "gui", this);
    
    NLua::RegisterClassFunction(LuaGetElfSym, "getsym", "elfclass");
    NLua::RegisterClassFunction(LuaGetElfSymVerDef, "getsymdef", "elfclass");
    NLua::RegisterClassFunction(LuaGetElfSymVerNeed, "getsymneed", "elfclass");
    
    // Set some default values for config variabeles
    NLua::LuaSet("", "appname", "cfg");
    
    NLua::CLuaTable tab("languages", "cfg");
    tab[1] << "english";
    tab[2] << "dutch";
    
    tab.Open("targetos", "cfg");
    tab[1] << m_szOS;
    
    tab.Open("targetarch", "cfg");
    tab[1] << m_szCPUArch;
    
    tab.Open("frontends", "cfg");
    tab[1] << "gtk";
    tab[2] << "fltk";
    tab[3] << "ncurses";
    
    tab.Close();
    
    NLua::LuaSet("lzma", "archivetype", "cfg");
    NLua::LuaSet("english", "defaultlang", "cfg");
    
    // Default pkg values
    NLua::LuaSet(false, "enable", "pkg");
    NLua::LuaSet(true, "register", "pkg");
    NLua::LuaSet(false, "setkdeenv", "pkg");
    NLua::LuaSet("1", "release", "pkg");
    NLua::LuaSet("", "url", "pkg");
    NLua::LuaSet("", "license", "pkg");
}

int CMain::m_iLuaDirIterCount = 0;

// Directory iter functions. Based on examples from "Programming in lua"
int CMain::LuaInitDirIter(lua_State *L)
{
    const char *path = luaL_checkstring(L, 1);
    
    if (m_iLuaDirIterCount > 5)
    {
        // HACK: Clean out any filedescriptors caused by this function.
        // As this function may be called in a loop, Lua's GC may not be quick enough
        // with collecting dir iters.
        lua_gc(L, LUA_GCCOLLECT, 0);
    }

    CDirIter *it = NULL;
    try
    {
        it = new CDirIter(path);
    }
    catch (Exceptions::CExOpenDir &e)
    {
        it = NULL;
        debugline("EXCEPTION: %s\n", e.what());
        // Don't do anything here, closure function will return nil
    }
    
    m_iLuaDirIterCount++;

    CDirIter **d = (CDirIter **)lua_newuserdata(L, sizeof(CDirIter *));
    *d = it;

    if (luaL_newmetatable(L, "diriter") == 1) // Table didn't exist yet?
    {
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, LuaDirIterGC);
        lua_settable(L, -3);
    }
    
    lua_setmetatable(L, -2);
    
    lua_pushcclosure(L, LuaDirIter, 1);
    return 1;
}

int CMain::LuaDirIter(lua_State *L)
{
    CDirIter *d = *(CDirIter **)lua_touserdata(L, lua_upvalueindex(1));
    
    if (d && *d)
    {
        // Get first valid entry
        while (*d && (!strcmp((*d)->d_name, ".") || !strcmp((*d)->d_name, "..")))
            (*d)++;

        if (!*d)
            return 0;
        
        lua_pushstring(L, (*d)->d_name);
        (*d)++;
        
        return 1;
    }
    else
        return 0;
}

int CMain::LuaDirIterGC(lua_State *L)
{
    CDirIter *d = *(CDirIter **)lua_touserdata(L, 1);
    delete d;
    m_iLuaDirIterCount--;
    return 0;
}

int CMain::LuaFileExists(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
    lua_pushboolean(L, FileExists(file));
    return 1;
}

int CMain::LuaReadPerm(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
    lua_pushboolean(L, ReadAccess(file));
    return 1;
}

int CMain::LuaWritePerm(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
    lua_pushboolean(L, WriteAccess(file));
    return 1;
}

int CMain::LuaIsDir(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
    lua_pushboolean(L, IsDir(file));
    return 1;
}

int CMain::LuaMKDir(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
    const char *modestr = luaL_optstring(L, 2, NULL);
    bool ignoreumask = lua_toboolean(L, 3), umaskchanged = false;
    mode_t dirmode, oldumask;

    try
    {
        if (modestr)
            dirmode = StrToMode(modestr);
        else
            dirmode = 0777; // This is the default mode for the mkdir command
    
        if (ignoreumask)
        {
            oldumask = umask(0);
            umaskchanged = true;
        }
        
        MKDir(file, dirmode);
        
        if (umaskchanged)
            umask(oldumask);
    }
    catch(Exceptions::CExMKDir &e)
    {
        if (umaskchanged)
            umask(oldumask);
        
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
    }

    lua_pushboolean(L, true);
    return 1;
}

int CMain::LuaMKDirRec(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
    const char *modestr = luaL_optstring(L, 2, NULL);
    bool ignoreumask = lua_toboolean(L, 3);
    mode_t dirmode, oldumask;

    try
    {
        if (modestr)
            dirmode = StrToMode(modestr);
        else
            dirmode = 0777; // This is the default mode for the mkdir command
    
        if (ignoreumask)
            oldumask = umask(0);
        
        size_t len = strlen(file), u = 0;
        while (true)
        {
            // Don't check if first char is '/', since that should exist anyway
            if ((u && (file[u] == '/')) || (u == (len-1)))
            {
                char tmp[u+2];
                strncpy(tmp, file, u+1);
                tmp[u+1] = 0;
                if (!FileExists(tmp))
                    MKDir(tmp, dirmode);
            }
            u++;
            
            if (u == len)
                break;
        }
        
        if (ignoreumask)
            umask(oldumask);
    }
    catch(Exceptions::CExMKDir &e)
    {
        if (ignoreumask)
            umask(oldumask);
        
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
    }
    
    lua_pushboolean(L, true);
    return 1;
}

int CMain::LuaCPFile(lua_State *L)
{
    std::list<const char *> srcfiles;
    const char *infile;
    int in, out;
    int args = lua_gettop(L);
     
    for (int u=1;u<args;u++)
    {
        infile = luaL_checkstring(L, u);
        if (infile)
            srcfiles.push_back(infile);
    }
    
    CPointerWrapper<char, void> dest(StrDup(luaL_checkstring(L, args)), free);
    
    // Strip trailing /'s
    size_t len = strlen(dest);
    while(dest && *dest && (dest[len-1] == '/'))
    {
        dest[len-1] = 0;
        len--;
    }

    if (srcfiles.empty() || !dest)
        luaL_error(L, "Bad source/destination files");
    
    bool isdir = IsDir(dest);
    if ((args >= 3) && !isdir)
    {
        if (!FileExists(dest))
            luaL_error(L, "Destination directory does not exist!");
        else
            luaL_error(L, "Destination has to be a directory when copying multiple files!");
    }

    for (std::list<const char *>::iterator it=srcfiles.begin(); it!=srcfiles.end(); it++)
    {
        in = open(*it, O_RDONLY);
        if (in < 0)
        {
            lua_pushnil(L);
            lua_pushfstring(L, "Could not open source file %s: %s\n", *it, strerror(errno));
            return 2;
        }
       
        CPointerWrapper<char, void> fname(StrDup(*it), free);
        CPointerWrapper<char, void> destfile((!isdir) ? StrDup(dest) : CreateTmpText("%s/%s", (char *)dest, basename(fname)), free);
        mode_t flags = O_WRONLY | (FileExists(destfile) ? O_TRUNC : O_CREAT);
        out = open(destfile, flags);

        if (out < 0)
        {
            lua_pushnil(L);
            lua_pushfstring(L, "Could not open destination file %s: %s\n", (char *)destfile, strerror(errno));
            close(in);
            return 2;
        }
        
        bool goterr = false;
        char buffer[1024];
        size_t size;
        while((size = read(in, buffer, sizeof(buffer))))
        {
            if (size < 0)
            {
                lua_pushnil(L);
                lua_pushfstring(L, "Error reading file %s: %s", srcfiles.front(), strerror(errno));
                goterr = true;
                break;
            }
            
            if (write(out, buffer, size) < 0)
            {
                lua_pushnil(L);
                lua_pushfstring(L, "Error writing to file %s: %s", (char *)destfile, strerror(errno));
                goterr = true;
                break;
            }
        }
        
        if (!goterr)
        {
            // Copy file permissions
            struct stat st; 
            if (fstat(in, &st) != 0)
            {
                lua_pushnil(L);
                lua_pushfstring(L, "Could not stat file %s", srcfiles.front());
                goterr = true;
            }
            else
            {
                // Disable umask temporary so we can normally copy permissions
                mode_t mask = umask(0);
                if (fchmod(out, st.st_mode) != 0)
                {
                    lua_pushnil(L);
                    lua_pushfstring(L, "Could not chmod destination file %s", (char *)destfile);
                    goterr = true;
                }
                umask(mask);
            }
        }
        close(in);
        close(out);

        if (goterr)
            return 2;
    }

    lua_pushboolean(L, true);
    return 1;
}

int CMain::LuaCHMod(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
    const char *modestr = luaL_checkstring(L, 2);
    mode_t fmode = StrToMode(modestr);
    
    if (chmod(file, fmode) != 0)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Could not chmod file %s: %s\n", file, strerror(errno));
        return 2;
    }
    
    lua_pushboolean(L, true);
    return 1;
}

int CMain::LuaGetCWD(lua_State *L)
{
    try
    {
        std::string curdir = GetCWD();
        lua_pushstring(L, curdir.c_str());
        return 1;
    }
    catch(Exceptions::CExReadDir &e)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Could not get current directory: %s", e.what());
        return 2;
    }
    
    return 0; // Not reached
}

int CMain::LuaCHDir(lua_State *L)
{
    const char *dir = luaL_checkstring(L, 1);
    
    try
    {
        CHDir(dir);
        lua_pushboolean(L, true);
        return 1;
    }
    catch(Exceptions::CExReadDir &e)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Could not change current directory: %s", e.what());
        return 2;
    }
    
    return 1;
}

int CMain::LuaGetFileSize(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
    struct stat st;
    
    if (lstat(file, &st) != 0)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Could not stat file %s: %s", file, strerror(errno));
        return 2;
    }
    
    lua_pushnumber(L, st.st_size);
    return 1;
}

int CMain::LuaLog(lua_State *L)
{
    std::string msg = luaL_checkstring(L, 1);
    int args = lua_gettop(L);
    
    for (int i=2; i<=args; i++)
        msg += luaL_checkstring(L, i);
    
    syslog(0, msg.c_str());
    debugline("log: %s\n", msg.c_str());
    
    return 0;
}

int CMain::LuaSetEnv(lua_State *L)
{
    const char *env = luaL_checkstring(L, 1);
    const char *val = luaL_checkstring(L, 2);
    bool overwrite = true;
    
    if (lua_isboolean(L, 3))
        overwrite = lua_toboolean(L, 3);
    
    setenv(env, val, overwrite);
    
    return 0;
}

int CMain::LuaYesNoBox(lua_State *L)
{
    CMain *pMain = (CMain *)lua_touserdata(L, lua_upvalueindex(1));
    std::string msg = luaL_checkstring(L, 1);
    int args = lua_gettop(L);
    
    for (int i=2; i<=args; i++)
        msg += luaL_checkstring(L, i);
    
    EscapeControls(msg);
    
    lua_pushboolean(L, pMain->YesNoBox(GetTranslation(msg.c_str())));
    
    return 1;
}

int CMain::LuaChoiceBox(lua_State *L)
{
    CMain *pMain = (CMain *)lua_touserdata(L, lua_upvalueindex(1));
    std::string msg = luaL_checkstring(L, 1);
    const char *but1 = luaL_checkstring(L, 2);
    const char *but2 = luaL_checkstring(L, 3);
    const char *but3 = lua_tostring(L, 4);
    
    EscapeControls(msg);
    
    if (but1)
        but1 = GetTranslation(but1);
    if (but2)
        but2 = GetTranslation(but2);
    if (but3)
        but3 = GetTranslation(but3);

    int ret = pMain->ChoiceBox(GetTranslation(msg.c_str()), but1, but2, but3) + 1; // +1: Convert 0-2 range to 1-3
    
    // UNDONE: FLTK doesn't return alternative number while ncurses returns -1.
    // For now we'll stick with the FLTK behaviour: return the last button.
    if (ret == 0) // Ncurses returns -1 and we added 1 thus zero...
        ret = 3;
    
    lua_pushinteger(L, ret);
    
    return 1;
}

int CMain::LuaWarnBox(lua_State *L)
{
    CMain *pMain = (CMain *)lua_touserdata(L, lua_upvalueindex(1));
    std::string msg = luaL_checkstring(L, 1);
    int args = lua_gettop(L);
    
    for (int i=2; i<=args; i++)
        msg += luaL_checkstring(L, i);
    
    EscapeControls(msg);
    
    pMain->WarnBox(GetTranslation(msg.c_str()));
    
    return 0;
}

int CMain::LuaMSGBox(lua_State *L)
{
    CMain *pMain = (CMain *)lua_touserdata(L, lua_upvalueindex(1));
    std::string msg = luaL_checkstring(L, 1);
    int args = lua_gettop(L);
    
    for (int i=2; i<=args; i++)
        msg += luaL_checkstring(L, i);
    
    EscapeControls(msg);
    
    pMain->MsgBox(GetTranslation(msg.c_str()));
    
    return 0;
}

int CMain::LuaExit(lua_State *L)
{
    throw Exceptions::CExUser();
    return 0; // Not reached
}

int CMain::LuaExitStatus(lua_State *L)
{
    int stat = luaL_checkint(L, 1);
    lua_pushinteger(L, WEXITSTATUS(stat));
    return 1;
}

int CMain::LuaMD5(lua_State *L)
{
    std::string md5 = GetMD5(luaL_checkstring(L, 1));
    
    if (md5 != "0")
        lua_pushstring(L, md5.c_str());
    else
        lua_pushnil(L);
        
    return 1;
}

int CMain::LuaTr(lua_State *L)
{
    const char *str = luaL_checkstring(L, 1);
    int args = lua_gettop(L);
    
    if (args == 1)
        lua_pushstring(L, GetTranslation(str));
    else if (args == 2)
        lua_pushstring(L, CreateText(GetTranslation(str), luaL_checkstring(L, 2)));
    else if (args == 3)
        lua_pushstring(L, CreateText(GetTranslation(str), luaL_checkstring(L, 2),
                       luaL_checkstring(L, 3)));
    else if (args == 4)
        lua_pushstring(L, CreateText(GetTranslation(str), luaL_checkstring(L, 2),
                       luaL_checkstring(L, 3), luaL_checkstring(L, 4)));
    else // Limitation of max 4 args ...
        lua_pushstring(L, CreateText(GetTranslation(str), luaL_checkstring(L, 2),
                       luaL_checkstring(L, 3), luaL_checkstring(L, 4), luaL_checkstring(L, 5)));
    
    return 1;
}

int CMain::LuaOpenElf(lua_State *L)
{
    CElfSymbolWrapper *elfw = NULL;
    try
    {
        elfw = new CElfSymbolWrapper(luaL_checkstring(L, 1));
    }
    catch (Exceptions::CExElf &e)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Could not create ELF class: %s\n", e.what());
        return 2;
    }
    
    NLua::CreateClass(elfw, "elfclass");
    NLua::SetClassGC("elfclass", LuaElfGC);
    
    return 1;
}

int CMain::LuaGetElfSym(lua_State *L)
{
    CElfSymbolWrapper *elfw = NLua::CheckClassData<CElfSymbolWrapper>("elfclass", 1);
    int n = luaL_checkint(L, 2) - 1; // Convert to 0 - size-1 range
    
    if ((n < 0) || (n >= static_cast<int>(elfw->GetSymSize())))
        return 0;
    
    TSTLVecSize index = SafeConvert<TSTLVecSize>(n);
    
    lua_newtable(L);
    int tab = lua_gettop(L);
    
    CElfSymbolWrapper::SSymData sym = elfw->GetSym(index);
    lua_pushstring(L, sym.name.c_str());
    lua_setfield(L, tab, "name");
    lua_pushstring(L, sym.binding.c_str());
    lua_setfield(L, tab, "binding");
    lua_pushstring(L, sym.version.c_str());
    lua_setfield(L, tab, "version");
    lua_pushboolean(L, sym.undefined);
    lua_setfield(L, tab, "undefined");
    
    return 1;
}

int CMain::LuaGetElfSymVerDef(lua_State *L)
{
    CElfSymbolWrapper *elfw = NLua::CheckClassData<CElfSymbolWrapper>("elfclass", 1);
    int n = luaL_checkint(L, 2) - 1; // Convert to 0 - size-1 range
    
    if ((n < 0) || (n >= static_cast<int>(elfw->GetSymVerDefSize())))
        return 0;
    
    TSTLVecSize index = SafeConvert<TSTLVecSize>(n);
    
    lua_newtable(L);
    int tab = lua_gettop(L);
    
    CElfSymbolWrapper::SVerSymData sym = elfw->GetSymVerDef(index);
    lua_pushstring(L, sym.name.c_str());
    lua_setfield(L, tab, "name");
    lua_pushstring(L, sym.flags.c_str());
    lua_setfield(L, tab, "flags");
    
    return 1;
}

int CMain::LuaGetElfSymVerNeed(lua_State *L)
{
    CElfSymbolWrapper *elfw = NLua::CheckClassData<CElfSymbolWrapper>("elfclass", 1);
    int n = luaL_checkint(L, 2) - 1; // Convert to 0 - size-1 range
    
    if ((n < 0) || (n >= static_cast<int>(elfw->GetSymVerNeedSize())))
        return 0;
    
    TSTLVecSize index = SafeConvert<TSTLVecSize>(n);
    
    lua_newtable(L);
    int tab = lua_gettop(L);
    
    CElfSymbolWrapper::SVerSymData sym = elfw->GetSymVerNeed(index);
    lua_pushstring(L, sym.name.c_str());
    lua_setfield(L, tab, "name");
    lua_pushstring(L, sym.flags.c_str());
    lua_setfield(L, tab, "flags");
    
    return 1;
}

int CMain::LuaElfGC(lua_State *L)
{
    CElfSymbolWrapper *elfw = NLua::CheckClassData<CElfSymbolWrapper>("elfclass", 1);
    delete elfw;
    return 0;
}

int CMain::LuaPanic(lua_State *L)
{
    throw Exceptions::CExLua(lua_tostring(L, 1));
    return 0;
}

// -------------------------------------
// Lua Runner Class
// -------------------------------------

void CLuaRunner::Init(int argc, char **argv)
{
    CMain::Init(argc, argv);
    
    NLua::LuaSet(argv[3], "maindir"); // Main nixstaller directory
    NLua::LuaSet(argv[4], "confdir"); // Project configuration directory
    NLua::LuaSet(((argc >= 6) ? argv[5] : "setup.sh"), "outname"); // Installer name
    NLua::LoadFile(argv[2]); // Script to execute
}
