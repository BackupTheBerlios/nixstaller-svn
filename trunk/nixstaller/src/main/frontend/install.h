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

class CPseudoTerminal;

class CBaseInstall: public CMain
{
    std::string m_CurLang, m_ConfigDir;
    std::string m_NixstDir; // For fastrun
    bool m_bFastRun;
    long m_lUpdateTimer;

    void ReadLang(void);

protected:
    void ParseTerminal(CPseudoTerminal &term, int luaout);
    const std::string &GetConfigDir(void) const { return m_ConfigDir; }
    const std::string &GetNixstDir(void) const { return m_NixstDir; }
    bool FastRun(void) const { return m_bFastRun; }

    virtual void CoreUpdateLanguage(void) = 0;
    virtual void InitLua(void);
    virtual void CoreUpdate(void) { } // Called during installation, lua execution etc
    
public:
    CBaseInstall(void) : m_bFastRun(false), m_lUpdateTimer(0) { }
    virtual ~CBaseInstall(void);
    
    void UpdateLanguage(void) { ReadLang(); CoreUpdateLanguage(); };
    void Update(void);
    
    virtual void Init(int argc, char **argv);

    static int LuaTr(lua_State *L);
    static int LuaUpdate(lua_State *L);
    static int LuaGetLang(lua_State *L);
    static int LuaSetLang(lua_State *L);
    static int LuaExecuteCMD(lua_State *L);
    static int LuaGetMacAppPath(lua_State *L);
    static int LuaInitDownload(lua_State *L);
    static int LuaProcessDownload(lua_State *L);
    static int LuaCloseDownload(lua_State *L);

    static int UpdateLuaDownloadProgress(void *clientp, double dltotal, double dlnow,
                                         double ultotal, double ulnow);
};


namespace Exceptions {

class CExFrontend: public CExMessage
{
public:
    CExFrontend(const char *msg) : CExMessage(msg) { };
};

class CExCommand: public CException
{
    char m_szCommand[512];
    
public:
    CExCommand(const char *cmd) { StoreString(cmd, m_szCommand, sizeof(m_szCommand)); };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not execute command: %s"), m_szCommand); };
};


}


#endif
