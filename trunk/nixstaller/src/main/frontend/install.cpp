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

#include <sys/stat.h>
#include <libgen.h>
#include <stdlib.h>

#include "main/pseudoterminal.h"
#include "main/lua/lua.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "main/frontend/curl.h"
#include "main/frontend/install.h"
#include "main/frontend/run.h"
#include "main/frontend/utils.h"

#if defined(__APPLE__)
// Include Location Services header files
#include <sys/param.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

std::map<std::string, char *> Translations;

// -------------------------------------
// Base Install Class
// -------------------------------------

CBaseInstall::~CBaseInstall()
{
    try
    {
        NLua::CLuaFunc luafunc("Finish");
        if (luafunc)
        {
            luafunc << HadError();
            luafunc(0);
        }
    }
    catch(Exceptions::CException &e)
    {
        HandleError();
    }
}

void CBaseInstall::ReadLang()
{
    FreeTranslations();
    
    std::ifstream file(CreateText("%s/lang/%s/strings", GetConfigDir().c_str(), m_CurLang.c_str()));

    if (!file)
    {
        debugline("WARNING: Failed to load language file for %s\n", m_CurLang.c_str());
        return;
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
}

void CBaseInstall::ParseTerminal(CPseudoTerminal &term, int luaout)
{
    NLua::CLuaFunc func(luaout, LUA_REGISTRYINDEX);
    if (!func)
        luaL_error(NLua::LuaState, "Error: could not use output function\n");
    
    while (term)
    {
        Update();
        if (term.CheckForData())
        {
            std::string line;
            CPseudoTerminal::EReadStatus ret = term.ReadLine(line);

            if (ret != CPseudoTerminal::READ_AGAIN)
            {
                if (ret == CPseudoTerminal::READ_LINE)
                    line += "\n";
                func << line;
                func(0);
            }
        }
    }
}

void CBaseInstall::InitLua()
{
    CMain::InitLua();

    NLua::LuaSet(m_ConfigDir, "configdir", "internal");
    NLua::LuaSet(m_NixstDir, "nixstdir", "internal");
    NLua::LuaSet(m_bFastRun, "fastrun", "internal");
    
    NLua::RegisterFunction(LuaTr, "tr");
    
    NLua::RegisterFunction(LuaUpdate, "update", "install", this);
    NLua::RegisterFunction(LuaGetLang, "getlang", "install", this);
    NLua::RegisterFunction(LuaExecuteCMD, "executecmd", "install", this);
    NLua::RegisterFunction(LuaGetMacAppPath, "getmacapppath", "install", this);

    NLua::RegisterFunction(LuaSetLang, "setlang", "internal", this);
    NLua::RegisterFunction(LuaInitDownload, "initdownload", "internal");

    NLua::RegisterClassFunction(LuaProcessDownload, "process", "downloadclass");
    NLua::RegisterClassFunction(LuaCloseDownload, "close", "downloadclass");
}

void CBaseInstall::Update()
{
    long curtime = GetTime();
    
    if (m_lUpdateTimer <= curtime)
    {
        m_lUpdateTimer = curtime + 10;
        CoreUpdate();
    }
}

void CBaseInstall::Init(int argc, char **argv)
{
    NLua::LuaSet(DirName(argv[0]), "bindir", "internal");

    for (int a=1; a<argc; a++)
    {
        if (!strcmp(argv[a], "-c"))
        {
            a++;
            if (a < argc)
            {
                m_ConfigDir = argv[a];
                MakeAbsolute(m_ConfigDir);
            }
            else
                throw Exceptions::CExUsage("Too less args for -c option");
        }
        else if (!strcmp(argv[a], "--fastrun"))
            m_bFastRun = true;
        else if (!strcmp(argv[a], "-n"))
        {
            a++;
            if (a < argc)
            {
                m_NixstDir = argv[a];
                MakeAbsolute(m_NixstDir);
            }
            else
                throw Exceptions::CExUsage("Too less args for -n option");
        }

    }

    if (m_ConfigDir.empty())
        throw Exceptions::CExUsage("No -c option given");
    else if (m_bFastRun && m_NixstDir.empty())
        throw Exceptions::CExUsage("No -n option given");

    CMain::Init(argc, argv);
    UpdateLanguage();

    // NOTE: Need to be after CMain::Init(), as this sets the run dir
    // UNDONE: Doesn't work for ncurses
    if (m_bFastRun)
        PrintIntro(JoinPath(m_NixstDir, "src/internal"));
    else
        PrintIntro(GetRunDir());
}

int CBaseInstall::LuaTr(lua_State *L)
{
    std::string ret = GetTranslation(luaL_checkstring(L, 1));
    int args = lua_gettop(L);
    
    for (int n=2; n<=args; n++)
    {
        TSTLStrSize pos = ret.find("%s");
        if (pos == std::string::npos)
            luaL_error(L, "Too few modifiers ('%s') given.");
        ret.replace(pos, 2, luaL_checkstring(L, n));
    }
    
    lua_pushstring(L, ret.c_str());
    return 1;
}

int CBaseInstall::LuaUpdate(lua_State *L)
{
    CBaseInstall *installer = NLua::GetFromClosure<CBaseInstall *>();
    installer->Update();
    return 0;
}

int CBaseInstall::LuaGetLang(lua_State *L)
{
    CBaseInstall *installer = NLua::GetFromClosure<CBaseInstall *>();
    lua_pushstring(L, installer->m_CurLang.c_str());
    return 1;
}

int CBaseInstall::LuaSetLang(lua_State *L)
{
    CBaseInstall *installer = NLua::GetFromClosure<CBaseInstall *>();
    installer->m_CurLang = luaL_checkstring(L, 1);
    installer->UpdateLanguage();
    return 0;
}

int CBaseInstall::LuaExecuteCMD(lua_State *L)
{
    CBaseInstall *installer = NLua::GetFromClosure<CBaseInstall *>();
    const char *cmd = luaL_checkstring(L, 1);
    
    luaL_checktype(NLua::LuaState, 2, LUA_TFUNCTION);
    int luaout = NLua::MakeReference(2);
    
    bool required = true;
    
    if (lua_isboolean(L, 3))
        required = lua_toboolean(L, 3);
    
    const char *path = lua_tostring(L, 4);

    int ret = 1;
    try
    {
        CPseudoTerminal term;

        if (path)
            term.SetPath(path);
        
        term.Exec(cmd);
        installer->ParseTerminal(term, luaout);
        ret = term.GetRetStatus();
        NLua::Unreference(luaout);

        if (required && (ret == 127))
            throw Exceptions::CExCommand(cmd);
    }
    catch (Exceptions::CExIO &)
    {
        NLua::Unreference(luaout);
        if (required)
            throw;
    }
    
    lua_pushinteger(L, ret);
    return 1;
}

int CBaseInstall::LuaGetMacAppPath(lua_State *L)
{
#if defined(__APPLE__)
    const char *bundleID = lua_tostring(L, 1);

    OSStatus rc;
    CFURLRef url = NULL;
    CFStringRef id = CFStringCreateWithBytes(NULL, 
                                             (const unsigned char *)bundleID, strlen(bundleID), 
                                              kCFStringEncodingUTF8, 0);
    rc = LSFindApplicationForInfo(kLSUnknownCreator, id, NULL, NULL, &url);
    CFRelease(id);
    if (rc == noErr) {
        char buf[MAXPATHLEN];
        if (CFURLGetFileSystemRepresentation(url, true, (unsigned char *)buf, MAXPATHLEN)) {
            if (strstr(buf, "/.Trash/")) {
                lua_pushnil(L);
                lua_pushfstring(L, "Found application in the Trash: %s",buf);
                return 2;
            } else {
                lua_pushstring(L, buf);
                return 1;
            }
        }
    }
    lua_pushnil(L);
    lua_pushstring(L,"Error Finding Application Path");
#else
    lua_pushnil(L);
    lua_pushstring(L,"GetMacAppPath can only be run on Mac OS X");
#endif
    return 2;
}

int CBaseInstall::LuaInitDownload(lua_State *L)
{
    const char *url = luaL_checkstring(L, 1);
    const char *dest = luaL_checkstring(L, 2);

    CCURLWrapper *curlw = NULL;
    try
    {
        curlw = new CCURLWrapper(url, dest);
        curlw->SetProgFunc(UpdateLuaDownloadProgress, curlw);
    }
    catch (Exceptions::CExCURL &e)
    {
        if (curlw)
            delete curlw;
        lua_pushnil(L);
        lua_pushfstring(L, "Could not create download class: %s\n", e.what());
        return 2;
    }
    catch (Exceptions::CExIO &e)
    {
        if (curlw)
            delete curlw;
        lua_pushnil(L);
        lua_pushfstring(L, "Could not open file: %s\n", e.what());
        return 2;
    }
    
    NLua::CreateClass(curlw, "downloadclass");
    
    return 1;
}

int CBaseInstall::LuaProcessDownload(lua_State *L)
{
    CCURLWrapper *curlw = NLua::CheckClassData<CCURLWrapper>("downloadclass", 1);
    bool done;
    
    try
    {
        done = !curlw->Process();
    }
    catch (Exceptions::CExCURL &e)
    {
        NLua::LuaPushError(e.what());
        return 2;
    }
    
    if (done)
    {
        lua_pushboolean(L, false);
        if (!curlw->Success())
        {
            lua_pushstring(L, curlw->ErrorMessage().c_str());
            return 2;
        }
    }
    else
        lua_pushboolean(L, true);
    
    return 1;
}

int CBaseInstall::LuaCloseDownload(lua_State *L)
{
    CCURLWrapper *curlw = NLua::CheckClassData<CCURLWrapper>("downloadclass", 1);
    NLua::DeleteClass(curlw, "downloadclass");
    delete curlw;
    return 0;
}

int CBaseInstall::UpdateLuaDownloadProgress(void *clientp, double dltotal, double dlnow,
                                            double ultotal, double ulnow)
{
    CCURLWrapper *curlw = static_cast<CCURLWrapper *>(clientp);
    NLua::CLuaFunc func("updateprogress", "downloadclass", curlw);
    if (func)
    {
        NLua::PushClass("downloadclass", curlw);
        func.PushData(); // Add 'self' to function argument list
        func << dltotal << dlnow;
        func(0);
    }
    return 0;
}
