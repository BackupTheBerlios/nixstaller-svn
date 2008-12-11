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

#include <fstream>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <poll.h>
#include <errno.h>

#include "main/main.h"
#include "main/install/basescreen.h"
#include "main/install/luacfgmenu.h"
#include "main/install/luacheckbox.h"
#include "main/install/luadepscreen.h"
#include "main/install/luadirselector.h"
#include "main/install/luagroup.h"
#include "main/install/luainput.h"
#include "main/install/lualabel.h"
#include "main/install/luamenu.h"
#include "main/install/luaprogressbar.h"
#include "main/install/luaprogressdialog.h"
#include "main/install/luaradiobutton.h"
#include "main/install/luatextfield.h"
#include "main/lua/lua.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "main/lua/luatable.h"

#if defined(__APPLE__)
// Include Location Services header files
#include <sys/param.h>
#include <ApplicationServices/ApplicationServices.h>
#endif
// -------------------------------------
// Base Installer Class
// -------------------------------------

CBaseInstall::CBaseInstall(void) : m_lUITimer(0), m_lRunTimer(0), m_pCurScreen(NULL),
                                   m_iAskQuitLuaFunc(LUA_NOREF)
{
}

void CBaseInstall::UpdateUI()
{
    long curtime = GetTime();
    
    if (m_lUITimer <= curtime)
    {
        m_lUITimer = curtime + 10;
        Run();
        CoreUpdateUI();
    }
}

void CBaseInstall::AddScreen(CBaseScreen *screen)
{
    m_ScreenList.push_back(screen);
    CoreAddScreen(screen);
}

void CBaseInstall::Init(int argc, char **argv)
{   
    m_szBinDir = dirname(CreateText(argv[0]));
    NLua::LuaSet(m_szBinDir, "bindir");
    
    CMain::Init(argc, argv); // Init main, will also read config files
    
    lua_pushlightuserdata(NLua::LuaState, this);
    lua_setfield(NLua::LuaState, LUA_REGISTRYINDEX, "installer");
    NLua::LuaSetHook(LuaHook);
    
    m_SUHandler.SetThinkFunc(SUThinkFunc, this);
}

void CBaseInstall::CoreUpdateLanguage()
{
    for (TScreenList::iterator it=m_ScreenList.begin(); it!=m_ScreenList.end(); it++)
        (*it)->UpdateLanguage();
}

int CBaseInstall::ExecuteCommand(const char *cmd, bool required, const char *path, int luaout)
{
    // Redirect stderr to stdout, so that errors will be displayed too
    const char *append = " 2>&1";
    char command[strlen(cmd) + strlen(append)];
    strcpy(command, cmd);
    strcat(command, append);
    
    if (path && *path)
        setenv("PATH", path, 1);
    else
        setenv("PATH", GetDefaultPath(), 1);
    
    NLua::CLuaFunc func(luaout, LUA_REGISTRYINDEX);
    if (!func)
        luaL_error(NLua::LuaState, "Error: could not use output function\n");
    
    CPipedCMD pipe(command);
    std::string line;
            
    while (pipe)
    {
        UpdateUI();
                
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
    
    return pipe.Close(required); // By calling Close() explicity its able to throw exceptions
}

int CBaseInstall::ExecuteCommandAsRoot(const char *cmd, bool required, const char *path, int luaout)
{
    GetSUPasswd("This installation requires root (administrator) privileges in order to continue.\n"
                "Please enter the administrative password below.", true);
    
    NLua::CLuaFunc func(luaout, LUA_REGISTRYINDEX);
    if (!func)
        luaL_error(NLua::LuaState, "Error: could not use output function\n");

    m_SUHandler.SetOutputFunc(CMDSUOutFunc, &func);

    if (!path || !path[0])
        path = GetDefaultPath();
    
    m_SUHandler.SetPath(path);
    m_SUHandler.SetCommand(cmd);
    
    if (!m_SUHandler.ExecuteCommand(m_szPassword))
    {
        if (required)
        {
            CleanPasswdString(m_szPassword);
            m_szPassword = NULL;
            throw Exceptions::CExCommand(cmd);
        }
    }
    
    m_SUHandler.SetOutputFunc(NULL);
    
    return m_SUHandler.Ret();
}

void CBaseInstall::SetDestDir(const char *dir)
{
    NLua::LuaSet(dir, "destdir", "install");
}

std::string CBaseInstall::GetDestDir(void)
{
    std::string ret;
    NLua::LuaGet(ret, "destdir", "install");
    return ret;
}

void CBaseInstall::InitLua()
{
    CMain::InitLua();
    
    CBaseScreen::LuaRegister();
    CBaseLuaGroup::LuaRegister();
    CBaseLuaWidget::LuaRegister();
    CBaseLuaInputField::LuaRegister();
    CBaseLuaCheckbox::LuaRegister();
    CBaseLuaRadioButton::LuaRegister();
    CBaseLuaDirSelector::LuaRegister();
    CBaseLuaCFGMenu::LuaRegister();
    CBaseLuaMenu::LuaRegister();
    CBaseLuaProgressBar::LuaRegister();
    CBaseLuaProgressDialog::LuaRegister();
    CBaseLuaTextField::LuaRegister();
    CBaseLuaLabel::LuaRegister();

    NLua::RegisterFunction(LuaNewScreen, "newscreen", "install", this);
    NLua::RegisterFunction(LuaAddScreen, "addscreen", "install", this);
    NLua::RegisterFunction(LuaGetTempDir, "gettempdir", "install", this);
    NLua::RegisterFunction(LuaGetPkgDir, "getpkgdir", "install", this);
    NLua::RegisterFunction(LuaGetMacAppPath, "getmacapppath", "install", this);
    NLua::RegisterFunction(LuaExecuteCMD, "executecmd", "install", this);
    NLua::RegisterFunction(LuaExecuteCMDAsRoot, "executecmdasroot", "install", this);
    NLua::RegisterFunction(LuaAskRootPW, "askrootpw", "install", this);
    NLua::RegisterFunction(LuaLockScreen, "lockscreen", "install", this);
    NLua::RegisterFunction(LuaVerifyDestDir, "verifydestdir", "install", this);
    NLua::RegisterFunction(LuaExtraFilesPath, "extrafilespath", "install", this);
    NLua::RegisterFunction(LuaGetLang, "getlang", "install", this);
    NLua::RegisterFunction(LuaSetLang, "setlang", "install", this);
    NLua::RegisterFunction(LuaUpdateUI, "updateui", "install", this);
    NLua::RegisterFunction(LuaSetAskQuit, "setaskquit", "install", this);
    NLua::RegisterFunction(LuaShowDepScreen, "showdepscreen", "install", this);
        
    NLua::RegisterFunction(LuaNewProgressDialog, "newprogressdialog", "gui", this);

    const char *env = getenv("HOME");
    if (env)
        SetDestDir(env);
    else
        SetDestDir("/");
    
    // Initialize variables
    NLua::CLuaTable("menuentries", "install").Close();
    
    NLua::LoadFile("config/config.lua");
    NLua::LoadFile("install.lua");

    NLua::CLuaTable table("languages", "cfg");
    
    if (table)
    {
        int size = table.Size();
        std::string lang;
        for (int i=1; i<=size; i++)
        {
            table[i] >> lang;
            m_Languages.push_back(lang);
        }
    }
    
    table.Close();
}

void CBaseInstall::DeleteScreens()
{
    debugline("Deleting %u screens...\n", m_ScreenList.size());
    while (!m_ScreenList.empty())
    {
        delete m_ScreenList.back();
        m_ScreenList.pop_back();
    }
}

void CBaseInstall::ActivateScreen(CBaseScreen *screen)
{
    m_pCurScreen = screen;
    screen->Activate();
}

void CBaseInstall::Run()
{
    long curtime = GetTime();
    
    if (m_lRunTimer <= curtime)
    {
        m_lRunTimer = curtime + 10;
        if (m_pCurScreen)
            m_pCurScreen->Update();
    }
}

bool CBaseInstall::VerifyDestDir(void)
{
    std::string dir = GetDestDir();
    
    if (FileExists(dir))
    {
        if (!ReadAccess(dir))
        {
            MsgBox(GetTranslation("You don't have read permissions for this directory.\n"
                                  "Please choose another directory or rerun this program as a user who has read permissions."));
            return false;
        }
        if (!WriteAccess(dir))
        {
            return (ChoiceBox(GetTranslation("You don't have write permissions for this directory.\n"
                                             "The files can be extracted as the root user,\n"
                                             "but you'll need to enter the administrative password for this later."),
                    GetTranslation("Choose another directory"),
                    GetTranslation("Continue as root"), NULL) == 1);
        }
    }
    else
    {
        // Create directory?
        if (YesNoBox(CreateText(GetTranslation("Directory %s does not exist, do you want to create it?"), dir.c_str())))
        {
            try
            {
                if (MKDirNeedsRoot(dir))
                {
                    GetSUPasswd("Your account doesn't have permissions to "
                            "create the directory.\nTo create it with the root "
                            "(administrator) account, please enter the administrative password below.", false);
                    MKDirRecRoot(dir, m_SUHandler, m_szPassword);
                }
                else
                    MKDirRec(dir);
            }
            catch(Exceptions::CExIO &e)
            {
                WarnBox(e.what());
                return false;
            }
            
            return true;
        }
        return false;
    }
    
    return true;
}

bool CBaseInstall::AskQuit()
{
    bool ret;
    NLua::CLuaFunc func(m_iAskQuitLuaFunc, LUA_REGISTRYINDEX);
    
    if (func)
    {
        func(1);
        func >> ret;
    }
    else
        luaL_error(NLua::LuaState, "Could not use askquit function");
    
    return ret;
}

void CBaseInstall::CMDSUOutFunc(const char *s, void *p)
{
    NLua::CLuaFunc *func = static_cast<NLua::CLuaFunc *>(p);
    (*func) << s;
    (*func)(0);
}

void CBaseInstall::LuaHook(lua_State *L, lua_Debug *ar)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "installer");
    CBaseInstall *pInstaller = static_cast<CBaseInstall *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
        
    try
    {
        pInstaller->UpdateUI();
    }
    catch(Exceptions::CException &e)
    {
        ConvertExToLuaError();
    }
}

int CBaseInstall::LuaNewScreen(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *name = luaL_optstring(L, 1, "");
    NLua::CreateClass(pInstaller->CreateScreen(name), "screen");
    return 1;
}

int CBaseInstall::LuaAddScreen(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    CBaseScreen *screen = NLua::CheckClassData<CBaseScreen>("screen", 1);
    pInstaller->AddScreen(screen);
    lua_pop(NLua::LuaState, 1);
    return 0;
}

int CBaseInstall::LuaNewProgressDialog(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    luaL_checktype(L, 1, LUA_TFUNCTION);
    int ref = NLua::MakeReference(1);
        
    CPointerWrapper<CBaseLuaProgressDialog> cl(pInstaller->CoreCreateProgDialog(ref));
    NLua::CreateClass(cl, "progressdialog");
    lua_pop(L, 1);
    cl->Run();
    NLua::DeleteClass(cl, "progressdialog");
    NLua::Unreference(ref);

    return 0;
}

int CBaseInstall::LuaShowDepScreen(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    luaL_checktype(L, 1, LUA_TFUNCTION);
    int ref = NLua::MakeReference(1);
    
    CPointerWrapper<CBaseLuaDepScreen> screen(pInstaller->CoreCreateDepScreen(ref));
    screen->Run();
    NLua::Unreference(ref);

    return 0;
}


int CBaseInstall::LuaGetTempDir(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *ret = CreateText("%s/tmp", pInstaller->m_szOwnDir.c_str());
    
    if (!FileExists(ret))
        MKDir(ret, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH));
    
    lua_pushstring(L, ret);
    return 1;
}

int CBaseInstall::LuaGetPkgDir(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *ret = CreateText("%s/pkg/files", pInstaller->m_szOwnDir.c_str());
    
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

int CBaseInstall::LuaExecuteCMD(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *cmd = luaL_checkstring(L, 1);
    
    luaL_checktype(NLua::LuaState, 2, LUA_TFUNCTION);
    int luaout = NLua::MakeReference(2);
    
    bool required = true;
    
    if (lua_isboolean(L, 3))
        required = lua_toboolean(L, 3);
    
    const char *path = lua_tostring(L, 4);
    
    lua_pushinteger(L, pInstaller->ExecuteCommand(cmd, required, path, luaout));
    NLua::Unreference(luaout);
    return 1;
}

int CBaseInstall::LuaExecuteCMDAsRoot(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *cmd = luaL_checkstring(L, 1);
    
    luaL_checktype(NLua::LuaState, 2, LUA_TFUNCTION);
    int luaout = NLua::MakeReference(2);
    
    bool required = true;
    
    if (lua_isboolean(L, 3))
        required = lua_toboolean(L, 3);
    
    const char *path = lua_tostring(L, 4);
    
    lua_pushinteger(L, pInstaller->ExecuteCommandAsRoot(cmd, required, path, luaout));
    NLua::Unreference(luaout);
    return 1;
}

int CBaseInstall::LuaAskRootPW(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    pInstaller->GetSUPasswd("This installation requires root (administrator) privileges in order to continue.\n"
        "Please enter the administrative password below.", true);
    return 0;
}

int CBaseInstall::LuaLockScreen(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    pInstaller->LockScreen(NLua::LuaToBool(1), NLua::LuaToBool(2), NLua::LuaToBool(3));
    return 0;
}

int CBaseInstall::LuaVerifyDestDir(lua_State *L)
{
    lua_pushboolean(L, GetFromClosure(L)->VerifyDestDir());
    return 1;
}

int CBaseInstall::LuaExtraFilesPath(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *file = luaL_optstring(L, 1, "");
    lua_pushfstring(L, "%s/files_extra/%s", pInstaller->m_szOwnDir.c_str(), file);
    return 1;
}

int CBaseInstall::LuaGetLang(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    lua_pushstring(L, pInstaller->m_szCurLang.c_str());
    return 1;
}

int CBaseInstall::LuaSetLang(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    pInstaller->m_szCurLang = luaL_checkstring(L, 1);
    pInstaller->UpdateLanguage();
    return 0;
}

int CBaseInstall::LuaUpdateUI(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    pInstaller->UpdateUI();
    return 0;
}

int CBaseInstall::LuaSetAskQuit(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    luaL_checktype(L, 1, LUA_TFUNCTION);
    
    if (pInstaller->m_iAskQuitLuaFunc != LUA_NOREF)
    {
        lua_rawgeti(NLua::LuaState, LUA_REGISTRYINDEX, pInstaller->m_iAskQuitLuaFunc);
        NLua::Unreference(pInstaller->m_iAskQuitLuaFunc);
    }
    
    pInstaller->m_iAskQuitLuaFunc = NLua::MakeReference(1);
    return 1;
}

// -------------------------------------
// Util functions
// -------------------------------------

CBaseInstall *GetFromClosure(lua_State *L)
{
    return reinterpret_cast<CBaseInstall *>(lua_touserdata(L, lua_upvalueindex(1)));
}
