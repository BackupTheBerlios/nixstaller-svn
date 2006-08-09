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

#include <fstream>
#include <sstream>
#include <limits>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <dirent.h>

#include "main.h"

std::list<char *> StringList;

// -------------------------------------
// Main Class
// -------------------------------------

CMain::~CMain()
{
    if (!m_Translations.empty())
    {
        for(std::map<std::string, char *>::iterator p=m_Translations.begin(); p!=m_Translations.end(); p++)
            delete [] (*p).second;
    }
    
    FreeStrings();    
}

bool CMain::Init(int argc, char **argv)
{
    // Initialize lua
    if (!m_LuaVM.Init())
        return false;

    // Get current OS and cpu arch name
    struct utsname inf;
    if (uname(&inf) == -1)
        return false;
    
    m_szOS = inf.sysname;
    std::transform(m_szOS.begin(), m_szOS.end(), m_szOS.begin(), tolower);

    m_szCPUArch = inf.machine;
    // Convert iX86 to x86
    if ((m_szCPUArch[0] == 'i') && (m_szCPUArch.compare(2, 2, "86") == 0))
        m_szCPUArch = "x86";

    char curdir[1024];
    if (getcwd(curdir, sizeof(curdir)) == 0)
        ThrowError(false, "Could not read current directory");

    m_szOwnDir = curdir;
    
    // Register some globals for lua
    m_LuaVM.RegisterString(m_szOS.c_str(), "osname", "os");
    m_LuaVM.RegisterString(m_szCPUArch.c_str(), "arch", "os");
    m_LuaVM.RegisterString(m_szOwnDir.c_str(), "curdir");
    m_LuaVM.RegisterFunction(LuaInitDirIter, "dir", "io");
    m_LuaVM.RegisterFunction(LuaFileExists, "fileexists", "io");
    m_LuaVM.RegisterFunction(LuaReadPerm, "readperm", "io");
    m_LuaVM.RegisterFunction(LuaWritePerm, "writeperm", "io");
    m_LuaVM.RegisterFunction(LuaIsDir, "isdir", "io");
    m_LuaVM.RegisterFunction(LuaMKDir, "mkdir", "io");
    m_LuaVM.RegisterFunction(LuaCPFile, "copy", "io");
    
    if (argc >= 4) // 3 arguments at least: "-c", the path to the lua script and the path to the project directory
    {
        if (!strcmp(argv[1], "-c"))
        {
            CreateInstall(argc, argv);
            EndProg();
        }
    }
     
    if (!ReadConfig())
        return false;
    
    if (m_Languages.empty())
    {
        char *s = new char[8];
        strcpy(s, "english");
        m_Languages.push_front(s);
    }
    
    m_szCurLang = m_Languages.front();

    return true;
}

void CMain::CreateInstall(int argc, char **argv)
{
    m_LuaVM.RegisterString(argv[3], "confdir");
    m_LuaVM.RegisterString(((argc >= 5) ? argv[4] : "setup.sh"), "outname");
    
    if (!m_LuaVM.LoadFile(argv[2]))
        ThrowError(false, "Error opening lua file!\n");
}

void CMain::ThrowError(bool dialog, const char *error, ...)
{
    char *txt;
    const char *translated = GetTranslation(error);
    va_list v;
    
    va_start(v, error);
        vasprintf(&txt, translated, v);
    va_end(v);

    if (dialog) Warn(txt);
    else { fprintf(stderr, GetTranslation("Error: %s"), txt); fprintf(stderr, "\n"); }

    EndProg(true);
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
            if (!m_szPassword)
            {
                if (ChoiceBox(GetTranslation("Root access is required to continue\nAbort installation?"),
                    GetTranslation("No"), GetTranslation("Yes"), NULL))
                    EndProg();
            }
            else
            {
                if (m_SUHandler.TestSU(m_szPassword))
                    break;

                // Some error appeared
                if (m_SUHandler.GetError() == LIBSU::CLibSU::SU_ERROR_INCORRECTPASS)
                    Warn(GetTranslation("Incorrect password given for root user\nPlease retype"));
                else
                {
                    ThrowError(true, GetTranslation("Could not use su to gain root access"
                            "Make sure you can use su(adding the current user to the wheel group may help"));
                }
            }
        }
    }
}

bool CMain::ReadLang()
{
    // Clear all translations
    if (!m_Translations.empty())
    {
        std::map<std::string, char *>::iterator p = m_Translations.begin();
        for(;p!=m_Translations.end();p++)
            delete [] (*p).second;
        m_Translations.erase(m_Translations.begin(), m_Translations.end());
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
            m_Translations[srcmsg] = new char[text.length()+1];
            text.copy(m_Translations[srcmsg], std::string::npos);
            m_Translations[srcmsg][text.length()] = 0;
        }

        atsrc = !atsrc;
    }

    return true;
}

std::string CMain::GetTranslation(std::string &s)
{
    std::map<std::string, char *>::iterator p = m_Translations.find(s);
    if (p != m_Translations.end())
        return (*p).second;
    
    // No translation found
    //debugline("WARNING: No translation for %s\n", s.c_str());
    return s;
}

char *CMain::GetTranslation(char *s)
{
    std::map<std::string, char *>::iterator p = m_Translations.find(s);
    if (p != m_Translations.end())
        return (*p).second;
    
    // No translation found
    //debugline("WARNING: No translation for %s\n", s);
    return s;
}

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

// Directory iter functions. Based on examples from "Programming in lua"
int CMain::LuaInitDirIter(lua_State *L)
{
    const char *path = luaL_checkstring(L, 1);
    
    DIR **d = (DIR **)lua_newuserdata(L, sizeof(DIR *));
    
    //luaL_getmetatable(L, "LuaBook.dir");
    if (luaL_newmetatable(L, "diriter") == 1) // Table didn't exist yet?
    {
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, LuaDirIterGC);
        lua_settable(L, -3);
    }
    
    lua_setmetatable(L, -2);
    
    *d = opendir(path);
    if (*d == NULL)
        luaL_error(L, "cannot open %s: %s", path, strerror(errno));
    
    lua_pushcclosure(L, LuaDirIter, 1);
    return 1;
}

int CMain::LuaDirIter(lua_State *L)
{
    DIR *d = *(DIR **)lua_touserdata(L, lua_upvalueindex(1));
    struct dirent *entry = readdir(d);
    
    while (entry && (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")))
        entry = readdir(d);
    
    if (entry)
    {
        lua_pushstring(L, entry->d_name);
        return 1;
    }
    else
        return 0;
}

int CMain::LuaDirIterGC(lua_State *L)
{
    DIR *d = *(DIR **)lua_touserdata(L, 1);
    if (d)
        closedir(d);
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
    bool ignoreumask = lua_toboolean(L, 3);
    mode_t dirmode, oldumask;

    if (modestr)
        dirmode = StrToMode(modestr);
    else
        dirmode = 0777; // This is the default mode for the mkdir command

    if (ignoreumask)
        oldumask = umask(0);
    
    if (mkdir(file, dirmode)) // Error
    {
        lua_pushstring(L, strerror(errno));
        umask(oldumask);
        return 1;
    }

    if (ignoreumask)
        umask(oldumask);

    return 0;
}

int CMain::LuaCPFile(lua_State *L)
{
    std::list<const char *> srcfiles;
    const char *infile;
    int in, out;
    unsigned args = lua_gettop(L);
     
    for (unsigned u=1;u<args;u++)
    {
        infile = luaL_checkstring(L, u);
        if (infile)
            srcfiles.push_back(infile);
    }
    
    char *dest = strdup(luaL_checkstring(L, args));
    
    // Strip trailing /'s
    unsigned len;
    while(dest && *dest && (dest[(len = strlen(dest))-1] == '/'))
        dest[len-1] = 0;

    if (srcfiles.empty() || !dest)
        luaL_error(L, "Bad source/destination files");
    
    bool isdir = IsDir(dest);
    if ((args >= 3) && !isdir)
    {
        lua_pushstring(L, "Destination has to be a directory when copying multiple files!");
        free(dest);
        return 1;
    }

    for (std::list<const char *>::iterator it=srcfiles.begin(); it!=srcfiles.end(); it++)
    {
        in = open(*it, O_RDONLY);
        if (in < 0)
        {
            lua_pushfstring(L, "Could not open source file %s: %s\n", srcfiles.front(), strerror(errno));
            free(dest);
            return 1;
        }
       
        char *destfile = (!isdir) ? dest : CreateTmpText("%s/%s", dest, *it);
        mode_t flags = O_WRONLY | (FileExists(destfile) ? O_TRUNC : O_CREAT);
        out = open(destfile, flags);

        if (out < 0)
        {
            lua_pushfstring(L, "Could not open destination file %s: %s\n", destfile, strerror(errno));
            close(in);
            free(destfile);
            return 1;
        }
        
        bool goterr = false;
        char buffer[1024];
        unsigned size;
        while((size = read(in, buffer, sizeof(buffer))))
        {
            if (size < 0)
            {
                lua_pushfstring(L, "Error reading file %s: %s", srcfiles.front(), strerror(errno));
                goterr = true;
                break;
            }
            
            if (write(out, buffer, size) < 0)
            {
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
                lua_pushfstring(L, "Could not stat file %s", srcfiles.front());
                goterr = true;
            }
            else
            {
                // Disable umask temporary so we can normally copy permissions
                mode_t mask = umask(0);
                if (fchmod(out, st.st_mode) != 0)
                {
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
            return 1;
    }

    return 0;
}

