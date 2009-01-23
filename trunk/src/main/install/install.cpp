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

#include "main/install/install.h"
#include "main/lua/lua.h"
#include "main/lua/luafunc.h"

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
    
    std::ifstream file(CreateText("%s/config/lang/%s/strings", GetOwnDir().c_str(), m_szCurLang.c_str()));

    if (!file)
    {
        debugline("WARNING: Failed to load language file for %s\n", m_szCurLang.c_str());
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

int CBaseInstall::ExecuteCommand(const char *cmd, bool required, const char *path, int luaout)
{
    // Redirect stderr to stdout, so that errors will be displayed too
    const char *append = " 2>&1";
    char command[strlen(cmd) + strlen(append)];
    strcpy(command, cmd);
    strcat(command, append);
    
    const char *oldpath = NULL;
    if (path && *path)
    {
        oldpath = getenv("PATH");
        setenv("PATH", path, 1);
    }
    
    NLua::CLuaFunc func(luaout, LUA_REGISTRYINDEX);
    if (!func)
        luaL_error(NLua::LuaState, "Error: could not use output function\n");
    
    CPipedCMD pipe(command);
    std::string line;
            
    while (pipe)
    {
        Update();
                
        if (pipe.HasData())
        {
            int ch = pipe.GetCh();
                    
            if (ch != EOF)
            {
                line += (char)ch;
                if ((char)ch == '\n')
                {
                    func << line;
                    func(0);
                    line.clear();
                }
            }
        }
    }
            
    if (!line.empty())
    {
        func << line.c_str();
        func(0);
    }
    
    if (oldpath)
        setenv("PATH", oldpath, 1);
    
    return pipe.Close(required); // By calling Close() explicity its able to throw exceptions
}

const char *CBaseInstall::GetLogoFName()
{
    std::string ret = "installer.png"; // Default
    NLua::LuaGet(ret, "logo", "cfg");
    return CreateText("%s/%s", GetOwnDir().c_str(), ret.c_str());
}

const char *CBaseInstall::GetAppIconFName()
{
    std::string ret = "appicon.xpm"; // Default
    NLua::LuaGet(ret, "appicon", "cfg");
    return CreateText("%s/%s", GetOwnDir().c_str(), ret.c_str());
}

const char *CBaseInstall::GetAboutFName()
{
    return CreateText("%s/about", GetOwnDir().c_str());
}

void CBaseInstall::InitLua()
{
    CMain::InitLua();
    
    NLua::RegisterFunction(LuaTr, "tr");
    
    NLua::RegisterFunction(LuaUpdate, "update", "install", this);
    NLua::RegisterFunction(LuaGetLang, "getlang", "install", this);
    NLua::RegisterFunction(LuaSetLang, "setlang", "install", this);
    NLua::RegisterFunction(LuaExecuteCMD, "executecmd", "install", this);
    NLua::RegisterFunction(LuaGetTempDir, "gettempdir", "install", this);
    NLua::RegisterFunction(LuaGetPkgDir, "getpkgdir", "install", this);
    NLua::RegisterFunction(LuaGetMacAppPath, "getmacapppath", "install", this);
    NLua::RegisterFunction(LuaExtraFilesPath, "extrafilespath", "install", this);
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
    NLua::LuaSet(dirname(CreateText(argv[0])), "bindir");
    CMain::Init(argc, argv);
//     NLua::LuaGet(m_szCurLang, "defaultlang", "cfg");
    UpdateLanguage();
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
    lua_pushstring(L, installer->m_szCurLang.c_str());
    return 1;
}

int CBaseInstall::LuaSetLang(lua_State *L)
{
    CBaseInstall *installer = NLua::GetFromClosure<CBaseInstall *>();
    installer->m_szCurLang = luaL_checkstring(L, 1);
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
    
    lua_pushinteger(L, installer->ExecuteCommand(cmd, required, path, luaout));
    NLua::Unreference(luaout);
    return 1;
}

int CBaseInstall::LuaGetTempDir(lua_State *L)
{
    CBaseInstall *installer = NLua::GetFromClosure<CBaseInstall *>();
    const char *ret = CreateText("%s/tmp", installer->GetOwnDir().c_str());
    
    if (!FileExists(ret))
        MKDir(ret, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH));
    
    lua_pushstring(L, ret);
    return 1;
}

int CBaseInstall::LuaGetPkgDir(lua_State *L)
{
    CBaseInstall *installer = NLua::GetFromClosure<CBaseInstall *>();
    const char *ret = CreateText("%s/pkg/files", installer->GetOwnDir().c_str());
    
    if (!FileExists(ret))
        MKDirRec(ret);
    
    const char *file = luaL_optstring(L, 1, "");
    lua_pushfstring(L, "%s/%s", ret, file);
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

int CBaseInstall::LuaExtraFilesPath(lua_State *L)
{
    CBaseInstall *installer = NLua::GetFromClosure<CBaseInstall *>();
    const char *file = luaL_optstring(L, 1, "");
    lua_pushfstring(L, "%s/files_extra/%s", installer->GetOwnDir().c_str(), file);
    return 1;
}
