/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <langinfo.h>

#include "main.h"
#include "elfutils.h"
#include "lua/lua.h"
#include "lua/luaclass.h"
#include "lua/luafunc.h"
#include "lua/luatable.h"

// -------------------------------------
// Main Class
// -------------------------------------

void CMain::Init(int argc, char **argv)
{
    // Get current OS and cpu arch name
    struct utsname inf;
    UName(inf);
    
    m_OS = inf.sysname;
    std::transform(m_OS.begin(), m_OS.end(), m_OS.begin(), tolower);

    m_CPUArch = inf.machine;
    if ((m_CPUArch[0] == 'i') && (m_CPUArch.compare(2, 2, "86") == 0))
        m_CPUArch = "x86";
    else if (m_CPUArch == "i86pc")
        m_CPUArch = "x86";
    else if (m_CPUArch == "amd64")
        m_CPUArch = "x86_64";

    m_OwnDir = GetCWD();

    for (int a=1; a<argc; a++)
    {
        if (!strcmp(argv[a], "-l"))
        {
            a++;
            if (a < argc)
                m_LuaDir = argv[a];
            else
                throw Exceptions::CExUsage("Too less args for -l option");
        }
        else if (!strcmp(argv[a], "--ls"))
        {
            a++;
            if (a < argc)
                m_LuaDirShared = argv[a];
            else
                throw Exceptions::CExUsage("Too less args for --ls option");
        }
    }

    if (m_LuaDir.empty())
        throw Exceptions::CExUsage("No -l option given");
    if (m_LuaDirShared.empty())
        throw Exceptions::CExUsage("No --ls option given");
    
    InitLua();
}

void CMain::InitLua()
{
    lua_atpanic(NLua::LuaState, LuaPanic);
    
    // Register some globals for lua
    NLua::LuaSet(m_OS, "osname", "os");
    NLua::LuaSet(m_CPUArch, "arch", "os");
    NLua::LuaSet(m_OwnDir, "curdir");
    NLua::LuaSet(m_LuaDir, "luasrcdir");
    NLua::LuaSet(m_LuaDirShared, "luasrcshdir");
    
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
    NLua::RegisterFunction(LuaGetEUID, "geteuid", "os");
    NLua::RegisterFunction(LuaHasUTF8, "hasutf8", "os");

    NLua::RegisterFunction(LuaAbort, "abort");
    
    NLua::RegisterClassFunction(LuaGetElfSym, "getsym", "elfclass");
    NLua::RegisterClassFunction(LuaGetElfSymVerDef, "getsymdef", "elfclass");
    NLua::RegisterClassFunction(LuaGetElfSymVerNeed, "getsymneed", "elfclass");
    NLua::RegisterClassFunction(LuaGetElfNeeded, "getneeded", "elfclass");
    NLua::RegisterClassFunction(LuaGetElfRPath, "getrpath", "elfclass");
    NLua::RegisterClassFunction(LuaGetElfMachine, "getmachine", "elfclass");
    NLua::RegisterClassFunction(LuaGetElfClass, "getclass", "elfclass");
    NLua::RegisterClassFunction(LuaCloseElf, "close", "elfclass");
    
    RunLuaShared("main.lua");
}

void CMain::RunLua(const char *script)
{
    NLua::LoadFile(JoinPath(m_LuaDir, script).c_str());
}

void CMain::RunLuaShared(const char *script)
{
    NLua::LoadFile(JoinPath(m_LuaDirShared, script).c_str());
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
        CPointerWrapper<char, void> fname(StrDup(*it), free);
        CPointerWrapper<char, void> destfile((!isdir) ? StrDup(dest) : CreateTmpText("%s/%s", (char *)dest, basename(fname)), free);

        if (!strcmp(*it, destfile))
            continue;
        
        in = open(*it, O_RDONLY);
        if (in < 0)
        {
            lua_pushnil(L);
            lua_pushfstring(L, "Could not open source file %s: %s\n", *it, strerror(errno));
            return 2;
        }
       
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
    const int args = lua_gettop(L);
    const char *ret;
    std::string msg;

    NLua::CLuaFunc tostring("tostring");
    if (tostring)
    {
        for (int i=1; i<=args; i++)
        {
            lua_pushvalue(L, i);
            tostring.PushData();
            if (tostring(1))
            {
                tostring >> ret;
                if (i > 1)
                    msg += "\t";
                msg += ret;
            }
        }
    }
    
    if (!msg.empty())
    {
        syslog(0, msg.c_str());
        debugline("log: %s\n", msg.c_str());
    }
    
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

int CMain::LuaOpenElf(lua_State *L)
{
    CElfWrapper *elfw = NULL;
    try
    {
        elfw = new CElfWrapper(luaL_checkstring(L, 1));
    }
    catch (Exceptions::CExElf &e)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Could not create ELF class: %s\n", e.what());
        return 2;
    }
    catch (Exceptions::CExIO &e)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "Could not open file: %s\n", e.what());
        return 2;
    }
    
    NLua::CreateClass(elfw, "elfclass");
    
    return 1;
}

int CMain::LuaGetElfSym(lua_State *L)
{
    CElfWrapper *elfw = NLua::CheckClassData<CElfWrapper>("elfclass", 1);
    int n = luaL_checkint(L, 2) - 1; // Convert to 0 - size-1 range
    
    if ((n < 0) || (n >= static_cast<int>(elfw->GetSymSize())))
        return 0;
    
    TSTLVecSize index = SafeConvert<TSTLVecSize>(n);
    
    lua_newtable(L);
    int tab = lua_gettop(L);
    
    CElfWrapper::SSymData sym = elfw->GetSym(index);
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
    CElfWrapper *elfw = NLua::CheckClassData<CElfWrapper>("elfclass", 1);
    int n = luaL_checkint(L, 2) - 1; // Convert to 0 - size-1 range
    
    if ((n < 0) || (n >= static_cast<int>(elfw->GetSymVerDefSize())))
        return 0;
    
    TSTLVecSize index = SafeConvert<TSTLVecSize>(n);
    
    lua_pushstring(L, elfw->GetSymVerDef(index).c_str());
    
    return 1;
}

int CMain::LuaGetElfSymVerNeed(lua_State *L)
{
    CElfWrapper *elfw = NLua::CheckClassData<CElfWrapper>("elfclass", 1);
    int n = luaL_checkint(L, 2) - 1; // Convert to 0 - size-1 range
    
    if ((n < 0) || (n >= static_cast<int>(elfw->GetSymVerNeedSize())))
        return 0;
    
    TSTLVecSize index = SafeConvert<TSTLVecSize>(n);
    
    lua_newtable(L);
    int tab = lua_gettop(L);
    
    CElfWrapper::SVerSymNeedData sym = elfw->GetSymVerNeed(index);
    lua_pushstring(L, sym.name.c_str());
    lua_setfield(L, tab, "name");
    lua_pushstring(L, sym.flags.c_str());
    lua_setfield(L, tab, "flags");
    lua_pushstring(L, sym.lib.c_str());
    lua_setfield(L, tab, "lib");

    return 1;
}

int CMain::LuaGetElfNeeded(lua_State *L)
{
    CElfWrapper *elfw = NLua::CheckClassData<CElfWrapper>("elfclass", 1);
    int n = luaL_checkint(L, 2) - 1; // Convert to 0 - size-1 range
    
    if ((n < 0) || (n >= static_cast<int>(elfw->GetNeededSize())))
        return 0;
    
    TSTLVecSize index = SafeConvert<TSTLVecSize>(n);
    
    lua_pushstring(L, elfw->GetNeeded(index).c_str());
    
    return 1;
}

int CMain::LuaGetElfRPath(lua_State *L)
{
    CElfWrapper *elfw = NLua::CheckClassData<CElfWrapper>("elfclass", 1);
    lua_pushstring(L, elfw->GetRPath().c_str());
    return 1;
}

int CMain::LuaGetElfMachine(lua_State *L)
{
    CElfWrapper *elfw = NLua::CheckClassData<CElfWrapper>("elfclass", 1);
    lua_pushinteger(L, elfw->GetMachine());
    return 1;
}

int CMain::LuaGetElfClass(lua_State *L)
{
    CElfWrapper *elfw = NLua::CheckClassData<CElfWrapper>("elfclass", 1);
    lua_pushstring(L, elfw->GetClass().c_str());
    return 1;
}

int CMain::LuaCloseElf(lua_State *L)
{
    CElfWrapper *elfw = NLua::CheckClassData<CElfWrapper>("elfclass", 1);
    NLua::DeleteClass(elfw, "elfclass");
    delete elfw;
    return 0;
}

int CMain::LuaPanic(lua_State *L)
{
    throw Exceptions::CExLua(lua_tostring(L, 1));
    return 0;
}

int CMain::LuaAbort(lua_State *L)
{
    throw Exceptions::CExLuaAbort(lua_tostring(L, 1));
    return 0;
}

int CMain::LuaGetEUID(lua_State *L)
{
    lua_pushnumber(L, geteuid());
    return 1;
}

int CMain::LuaHasUTF8(lua_State *L)
{
    // From: http://www.cl.cam.ac.uk/~mgk25/unicode.html
    lua_pushboolean(L, (strcmp(nl_langinfo(CODESET), "UTF-8") == 0));
    return 1;
}
