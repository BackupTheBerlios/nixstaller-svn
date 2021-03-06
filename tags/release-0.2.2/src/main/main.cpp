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

#include <fstream>
#include <sstream>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>

#include "main.h"

std::list<char *> StringList;
std::map<std::string, char *> Translations;

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    PrintIntro();

    bool runscript = ((argc >= 4) && !strcmp(argv[1], "-c")); // Caller (usually geninstall.sh) wants to run a lua script?
    int ret = EXIT_FAILURE;
    
    try
    {
        if (runscript)
        {
            CLuaRunner lr;
            lr.Init(argc, argv);
        }
        else
        {
            StartFrontend(argc, argv);
        }
        ret = EXIT_SUCCESS;
    }
    catch(Exceptions::CExUser &e)
    {
        debugline("User cancel\n");
    }
    catch(Exceptions::CException &e)
    {
        if (runscript)
            fprintf(stderr, GetTranslation(e.what())); // No specific way to complain, just use stderr
        else
            ReportError(GetTranslation(e.what()));
    }
    
    if (!runscript)
        StopFrontend();
    
    FreeStrings();
    return ret;
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
    // Convert iX86 to x86
    if ((m_szCPUArch[0] == 'i') && (m_szCPUArch.compare(2, 2, "86") == 0))
        m_szCPUArch = "x86";
    else if (m_szCPUArch == "i86pc")
        m_szCPUArch = "x86";

    m_szOwnDir = GetCWD();
    
    // Initialize lua
    m_LuaVM.Init();
    InitLua();
    
    if (m_Languages.empty())
    {
        char *s = new char[8];
        strcpy(s, "english");
        m_Languages.push_back(s);
    }
    
    if (!m_LuaVM.GetStrVar(&m_szCurLang, "defaultlang", "cfg"))
        m_szCurLang = "english";
    
    if (std::find(m_Languages.begin(), m_Languages.end(), m_szCurLang) == m_Languages.end())
        m_szCurLang = m_Languages.front();
    
    debugline("defaultlang: %s\n", m_szCurLang.c_str());
    ReadLang();
}

void CMain::SetUpSU(const char *msg)
{
    m_SUHandler.SetUser("root");
    m_SUHandler.SetTerminalOutput(false);

    if (m_SUHandler.NeedPassword())
    {
        while(true)
        {
            CleanPasswdString(m_szPassword);
            
            m_szPassword = GetPassword(GetTranslation(msg));
            
            // Check if password is invalid
            if (!m_szPassword || !m_szPassword[0])
            {
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
                    WarnBox(GetTranslation("Incorrect password given for root user\nPlease retype"));
                else
                {
                    throw Exceptions::CExSU("Could not use su to gain root access"
                                            "Make sure you can use su (adding the current user to the wheel group may help)");
                }
            }
        }
    }
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
    
    std::ifstream file(CreateText("config/lang/%s/strings", m_szCurLang.c_str()));

    if (!file)
        return false;
    
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
    // Register some globals for lua
    m_LuaVM.RegisterString("0.2.2", "version");
    
    m_LuaVM.RegisterString(m_szOS.c_str(), "osname", "os");
    m_LuaVM.RegisterString(m_szCPUArch.c_str(), "arch", "os");
    m_LuaVM.RegisterString(m_szOwnDir.c_str(), "curdir");
    
    m_LuaVM.RegisterFunction(LuaInitDirIter, "dir", "io");
    
    m_LuaVM.RegisterFunction(LuaFileExists, "fileexists", "os");
    m_LuaVM.RegisterFunction(LuaReadPerm, "readperm", "os");
    m_LuaVM.RegisterFunction(LuaWritePerm, "writeperm", "os");
    m_LuaVM.RegisterFunction(LuaIsDir, "isdir", "os");
    m_LuaVM.RegisterFunction(LuaMKDir, "mkdir", "os");
    m_LuaVM.RegisterFunction(LuaMKDirRec, "mkdirrec", "os");
    m_LuaVM.RegisterFunction(LuaCPFile, "copy", "os");
    m_LuaVM.RegisterFunction(LuaCHMod, "chmod", "os");
    m_LuaVM.RegisterFunction(LuaGetCWD, "getcwd", "os");
    m_LuaVM.RegisterFunction(LuaCHDir, "chdir", "os");
    m_LuaVM.RegisterFunction(LuaGetFileSize, "filesize", "os");
    m_LuaVM.RegisterFunction(LuaLog, "log", "os", this);
    m_LuaVM.RegisterFunction(LuaSetEnv, "setenv", "os", this);
    
    m_LuaVM.RegisterFunction(LuaMSGBox, "msgbox", "gui", this);
    m_LuaVM.RegisterFunction(LuaYesNoBox, "yesnobox", "gui", this);
    m_LuaVM.RegisterFunction(LuaChoiceBox, "choicebox", "gui", this);
    m_LuaVM.RegisterFunction(LuaWarnBox, "warnbox", "gui", this);
    
    m_LuaVM.RegisterFunction(LuaExit, "exit"); // Override
    
    // Set some default values for config variabeles
    m_LuaVM.SetArrayStr("english", "languages", 1, "cfg");
    m_LuaVM.SetArrayStr("dutch", "languages", 2, "cfg");
    m_LuaVM.SetArrayStr(m_szOS.c_str(), "targetos", 1, "cfg");
    m_LuaVM.SetArrayStr(m_szCPUArch.c_str(), "targetarch", 1, "cfg");
    m_LuaVM.SetArrayStr("fltk", "frontends", 1, "cfg");
    m_LuaVM.SetArrayStr("ncurses", "frontends", 2, "cfg");
    m_LuaVM.RegisterString("lzma", "archivetype", "cfg");
    m_LuaVM.RegisterString("english", "defaultlang", "cfg");
}

// Directory iter functions. Based on examples from "Programming in lua"
int CMain::LuaInitDirIter(lua_State *L)
{
    const char *path = luaL_checkstring(L, 1);
    
    CDirIter *it = NULL;
    try
    {
        it = new CDirIter(path);
    }
    catch (Exceptions::CExOpenDir &e)
    {
//         lua_pushnil(L);
//         lua_pushstring(L, e.what());
//         return 2;
        // Don't do anything here, closure function will fail
    }
    
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
    
    char *dest = StrDup(luaL_checkstring(L, args));
    
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
        free(dest);
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
            free(dest);
            return 2;
        }
       
        char *destfile = (!isdir) ? dest : CreateTmpText("%s/%s", dest, basename(((char *)*it)));
        mode_t flags = O_WRONLY | (FileExists(destfile) ? O_TRUNC : O_CREAT);
        out = open(destfile, flags);

        if (out < 0)
        {
            lua_pushnil(L);
            lua_pushfstring(L, "Could not open destination file %s: %s\n", destfile, strerror(errno));
            close(in);
            free(destfile);
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
                lua_pushfstring(L, "Error writing to file %s: %s", destfile, strerror(errno));
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
                    lua_pushfstring(L, "Could not chmod destination file %s", destfile);
                    goterr = true;
                }
                umask(mask);
            }
        }
        close(in);
        close(out);
        free(destfile);

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
        lua_pushfstring(L, "Could not chmod file %s: %s\n", file);
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
    
    lua_pushboolean(L, pMain->YesNoBox(msg.c_str()));
    
    return 1;
}

int CMain::LuaChoiceBox(lua_State *L)
{
    CMain *pMain = (CMain *)lua_touserdata(L, lua_upvalueindex(1));
    const char *msg = luaL_checkstring(L, 1);
    const char *but1 = luaL_checkstring(L, 2);
    const char *but2 = luaL_checkstring(L, 3);
    const char *but3 = lua_tostring(L, 4);
    
    int ret = pMain->ChoiceBox(msg, but1, but2, but3) + 1; // +1: Convert 0-2 range to 1-3
    
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
    
    pMain->WarnBox(msg.c_str());
    
    return 0;
}

int CMain::LuaMSGBox(lua_State *L)
{
    CMain *pMain = (CMain *)lua_touserdata(L, lua_upvalueindex(1));
    std::string msg = luaL_checkstring(L, 1);
    int args = lua_gettop(L);
    
    for (int i=2; i<=args; i++)
        msg += luaL_checkstring(L, i);
    
    pMain->MsgBox(msg.c_str());
    
    return 0;
}

int CMain::LuaExit(lua_State *L)
{
    throw Exceptions::CExUser();
    return 0; // Not reached
}

// -------------------------------------
// Lua Runner Class
// -------------------------------------

void CLuaRunner::CreateInstall(int argc, char **argv)
{
    m_LuaVM.RegisterString(argv[3], "confdir");
    m_LuaVM.RegisterString(((argc >= 5) ? argv[4] : "setup.sh"), "outname");
    m_LuaVM.LoadFile(argv[2]);
}


void CLuaRunner::Init(int argc, char **argv)
{
    CMain::Init(argc, argv);
    CreateInstall(argc, argv);
}
