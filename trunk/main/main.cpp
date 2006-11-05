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
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>

#include "main.h"

std::list<char *> StringList;
std::map<std::string, char *> Translations;

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    printf("Nixstaller version 0.2, Copyright (C) 2006 of Rick Helmus\n"
           "Nixstaller comes with ABSOLUTELY NO WARRANTY.\n"
           "This is free software, and you are welcome to redistribute it\n"
           "under certain conditions; see the about section for details.\n");
    
    // Caller (usually geninstall.sh) wants to run a lua script?
    if ((argc >= 4) && !strcmp(argv[1], "-c"))
    {
        // HACK: Run lua script and exit
        CLuaRunner *p = new CLuaRunner;
        p->Init(argc, argv);
        delete p;
        EndProg();
    }
    else
        return RunFrontend(argc, argv);
    
    return EXIT_SUCCESS; // Never reached
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
    
    FreeStrings();
    closelog();
}

bool CMain::Init(int argc, char **argv)
{
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
    
    // Initialize lua
    if (!m_LuaVM.Init())
        return false;

    if (!InitLua())
        return false;
    
/*    if (argc >= 4) // 3 arguments at least: "-c", the path to the lua script and the path to the project directory
    {
        if (!strcmp(argv[1], "-c"))
        {
            printf("Switching to script runner\n");

            CreateInstall(argc, argv);
            EndProg();
        }
    }*/
     
    if (m_Languages.empty())
    {
        char *s = new char[8];
        strcpy(s, "english");
        m_Languages.push_back(s);
    }
    
    if (!m_LuaVM.GetStrVar(&m_szCurLang, "defaultlang") ||
        (std::find(m_Languages.begin(), m_Languages.end(), m_szCurLang) == m_Languages.end()))
        m_szCurLang = "english"; // Default to english if wrong or unknown language is specified
    
    debugline("defaultlang: %s\n", m_szCurLang.c_str());
    ReadLang();

    return true;
}

void CMain::ThrowError(bool dialog, const char *error, ...)
{
    char *txt;
    const char *translated = GetTranslation(error);
    va_list v;
    
    va_start(v, error);
        vasprintf(&txt, translated, v);
    va_end(v);

    if (dialog) WarnBox(txt);
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
                    WarnBox(GetTranslation("Incorrect password given for root user\nPlease retype"));
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

bool CMain::InitLua()
{
    // Register some globals for lua
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
    m_LuaVM.RegisterFunction(LuaMSGBox, "MSGBox", NULL, this);
    m_LuaVM.RegisterFunction(LuaLog, "Log", NULL, this);
    
    // Set some default values for config variabeles
    m_LuaVM.SetArrayStr("english", "languages", 1);
    m_LuaVM.SetArrayStr(m_szOS.c_str(), "targetos", 1);
    m_LuaVM.SetArrayStr(m_szCPUArch.c_str(), "targetarch", 1);
    m_LuaVM.SetArrayStr("fltk", "frontends", 1);
    m_LuaVM.SetArrayStr("ncurses", "frontends", 2);
    m_LuaVM.RegisterString("gzip", "archivetype");
    m_LuaVM.RegisterString("english", "defaultlang");
    
    return true;
}

// Directory iter functions. Based on examples from "Programming in lua"
int CMain::LuaInitDirIter(lua_State *L)
{
    const char *path = luaL_checkstring(L, 1);
    
    DIR **d = (DIR **)lua_newuserdata(L, sizeof(DIR *));
    
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
    
    int ret = mkdir(file, dirmode);
    
    if (ignoreumask)
        umask(oldumask);

    if (ret != 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, strerror(errno));
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
            {
                if (mkdir(tmp, dirmode) != 0)
                {
                    if (ignoreumask)
                        umask(oldumask);

                    lua_pushnil(L);
                    lua_pushfstring(L, "Could not create directory %s: %s", tmp, strerror(errno));
                    return 2;
                }
            }
        }
        u++;
        
        if (u == len)
            break;
    }
    
    if (ignoreumask)
        umask(oldumask);

    lua_pushboolean(L, true);
    return 1;
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
    // UNDONE: Check if dest is NULL
    
    // Strip trailing /'s
    unsigned len = strlen(dest);
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
        unsigned size;
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
    char curdir[1024];
    if (getcwd(curdir, sizeof(curdir)) == 0)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Could not get current directory: %s", strerror(errno));
        return 2;
    }
    
    lua_pushstring(L, curdir);
    return 1;
}

int CMain::LuaCHDir(lua_State *L)
{
    const char *dir = luaL_checkstring(L, 1);
    if (chdir(dir) != 0)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Could not change current directory: %s", strerror(errno));
        return 2;
    }
    
    lua_pushboolean(L, true);
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

int CMain::LuaLog(lua_State *L)
{
    std::string msg = luaL_checkstring(L, 1);
    int args = lua_gettop(L);
    
    for (int i=2; i<=args; i++)
        msg += luaL_checkstring(L, i);
    
    syslog(0, msg.c_str());
    
    return 0;
}

// -------------------------------------
// Lua Runner Class
// -------------------------------------

void CLuaRunner::CreateInstall(int argc, char **argv)
{
    m_LuaVM.RegisterString(argv[3], "confdir");
    m_LuaVM.RegisterString(((argc >= 5) ? argv[4] : "setup.sh"), "outname");
    
    if (!m_LuaVM.LoadFile(argv[2]))
        ThrowError(false, "\nError parsing lua file!\n");
}


bool CLuaRunner::Init(int argc, char **argv)
{
    if (!CMain::Init(argc, argv))
        return false;
    
    CreateInstall(argc, argv);
    
    return true;
}
