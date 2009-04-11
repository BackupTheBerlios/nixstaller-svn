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

#ifndef MAIN_H
#define MAIN_H

#include <assert.h>
#include <syslog.h>
#include <stdarg.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <limits>
#include <fstream>
        
typedef std::vector<std::string>::size_type TSTLVecSize;
typedef std::string::size_type TSTLStrSize;

#include "exception.h"

#ifndef HAVE_VASPRINTF
extern "C" int vasprintf (char **result, const char *format, va_list args);
#endif

#include "main/lua/lua.h"

class CMain
{
    static int m_iLuaDirIterCount;
    std::string m_OS, m_CPUArch, m_OwnDir, m_LuaDir, m_LuaDirShared;

protected:
    const std::string &GetOwnDir(void) const { return m_OwnDir; }
    
    virtual void InitLua(void);

    void SetLuaDir(const char *dir) { m_LuaDir = dir; }
    void SetLuaDirSh(const char *dir) { m_LuaDirShared = dir; }
    void RunLua(const char *script);
    void RunLuaShared(const char *script);
    
public:
    CMain(void) { openlog("Nixstaller", LOG_USER|LOG_INFO, LOG_USER|LOG_INFO); };
    virtual ~CMain(void) { closelog(); }
    
    virtual void Init(int argc, char **argv);
    
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
    static int LuaLog(lua_State *L);
    static int LuaSetEnv(lua_State *L);
    static int LuaExit(lua_State *L);
    static int LuaExitStatus(lua_State *L);
    static int LuaMD5(lua_State *L);
    static int LuaOpenElf(lua_State *L);
    static int LuaGetElfSym(lua_State *L);
    static int LuaGetElfSymVerDef(lua_State *L);
    static int LuaGetElfSymVerNeed(lua_State *L);
    static int LuaGetElfNeeded(lua_State *L);
    static int LuaGetElfRPath(lua_State *L);
    static int LuaGetElfMachine(lua_State *L);
    static int LuaGetElfClass(lua_State *L);
    static int LuaCloseElf(lua_State *L);
    static int LuaElfGC(lua_State *L);
    static int LuaPanic(lua_State *L);
    static int LuaAbort(lua_State *L);
    static int LuaGetEUID(lua_State *L);
    static int LuaHasUTF8(lua_State *L);
};

#include "utils.h"

// #define RELEASE /* Enable on a release build */

#ifdef RELEASE
inline void debugline(const char *, ...) { };
#else
void debugline(const char *t, ...); // Defined in frontend code
#endif

#endif
