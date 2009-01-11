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

#ifndef INSTALL_H
#define INSTALL_H

#include "main/main.h"

class CBaseInstall: public CMain
{
    std::string m_szCurLang;
    long m_lUpdateTimer;

    void ReadLang(void);
    int ExecuteCommand(const char *cmd, bool required, const char *path, int luaout);

protected:
    const char *GetLogoFName(void);
    const char *GetAppIconFName(void);
    const char *GetAboutFName(void);

    virtual void CoreUpdateLanguage(void) = 0;
    virtual void InitLua(void);
    virtual void CoreUpdate(void) { } // Called during installation, lua execution etc
    
public:
    CBaseInstall(void) : m_lUpdateTimer(0) { }
    virtual ~CBaseInstall(void);
    
    void UpdateLanguage(void) { ReadLang(); CoreUpdateLanguage(); };
    void Update(void);
    
    virtual void Init(int argc, char **argv);

    static int LuaTr(lua_State *L);
    static int LuaUpdate(lua_State *L);
    static int LuaGetLang(lua_State *L);
    static int LuaSetLang(lua_State *L);
    static int LuaExecuteCMD(lua_State *L);
    static int LuaGetTempDir(lua_State *L);
    static int LuaGetPkgDir(lua_State *L);
    static int LuaGetMacAppPath(lua_State *L);
    static int LuaExtraFilesPath(lua_State *L);
};

#endif
