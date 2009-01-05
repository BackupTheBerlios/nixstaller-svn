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

#include "main/install/attinstall.h"
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
#include "main/lua/luafunc.h"


// -------------------------------------
// Base Attended Installer Class
// -------------------------------------

CBaseAttInstall::CBaseAttInstall(void) : m_szPassword(NULL), m_pCurScreen(NULL), m_iAskQuitLuaFunc(LUA_NOREF)
{
}

bool CBaseAttInstall::GetSUPasswd(const char *msg, bool mandatory)
{
    if ((!m_szPassword || !m_szPassword[0]) && m_SUHandler.NeedPassword())
    {
        while(true)
        {
            CleanPasswdString(m_szPassword);
            
            m_szPassword = GetPassword(GetTranslation(msg));
            
            // Check if password is invalid
            if (!m_szPassword || !m_szPassword[0])
            {
                if (!mandatory)
                    return false;
                
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
                    WarnBox(GetTranslation("Incorrect password given, please retype."));
                else
                {
                    const char *msg = "Could not use su or sudo to gain root access.";
                    
                    if (mandatory)
                        throw Exceptions::CExSU(msg);
                    else
                    {
                        WarnBox(GetTranslation(msg));
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

void CBaseAttInstall::AddScreen(CBaseScreen *screen)
{
    m_ScreenList.push_back(screen);
    CoreAddScreen(screen);
}

void CBaseAttInstall::Init(int argc, char **argv)
{   
    CBaseInstall::Init(argc, argv); // Init main, will also read config files
    
    lua_pushlightuserdata(NLua::LuaState, this);
    lua_setfield(NLua::LuaState, LUA_REGISTRYINDEX, "installer");
    NLua::LuaSetHook(LuaHook);
    
    m_SUHandler.SetUser("root");
    m_SUHandler.SetTerminalOutput(false);
    m_SUHandler.SetThinkFunc(SUThinkFunc, this);
}

void CBaseAttInstall::CoreUpdateLanguage()
{
    for (TScreenList::iterator it=m_ScreenList.begin(); it!=m_ScreenList.end(); it++)
        (*it)->UpdateLanguage();
}

void CBaseAttInstall::CoreUpdate()
{
    if (m_pCurScreen)
        m_pCurScreen->Update();
}

int CBaseAttInstall::ExecuteCommandAsRoot(const char *cmd, bool required, const char *path, int luaout)
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

std::string CBaseAttInstall::GetDestDir(void)
{
    std::string ret;
    NLua::LuaGet(ret, "destdir", "install");
    return ret;
}

void CBaseAttInstall::InitLua()
{
    CBaseInstall::InitLua();
    
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
    NLua::RegisterFunction(LuaExecuteCMDAsRoot, "executecmdasroot", "install", this);
    NLua::RegisterFunction(LuaAskRootPW, "askrootpw", "install", this);
    NLua::RegisterFunction(LuaLockScreen, "lockscreen", "install", this);
    NLua::RegisterFunction(LuaVerifyDestDir, "verifydestdir", "install", this);
    NLua::RegisterFunction(LuaSetAskQuit, "setaskquit", "install", this);
    NLua::RegisterFunction(LuaShowDepScreen, "showdepscreen", "install", this);
    
    NLua::RegisterFunction(LuaMSGBox, "msgbox", "gui", this);
    NLua::RegisterFunction(LuaYesNoBox, "yesnobox", "gui", this);
    NLua::RegisterFunction(LuaChoiceBox, "choicebox", "gui", this);
    NLua::RegisterFunction(LuaWarnBox, "warnbox", "gui", this);
    NLua::RegisterFunction(LuaNewProgressDialog, "newprogressdialog", "gui", this);

    NLua::LoadFile("install.lua");
}

void CBaseAttInstall::DeleteScreens()
{
    debugline("Deleting %u screens...\n", m_ScreenList.size());
    while (!m_ScreenList.empty())
    {
        delete m_ScreenList.back();
        m_ScreenList.pop_back();
    }
}

void CBaseAttInstall::ActivateScreen(CBaseScreen *screen)
{
    m_pCurScreen = screen;
    screen->Activate();
}

bool CBaseAttInstall::VerifyDestDir(void)
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

bool CBaseAttInstall::AskQuit()
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

void CBaseAttInstall::CMDSUOutFunc(const char *s, void *p)
{
    NLua::CLuaFunc *func = static_cast<NLua::CLuaFunc *>(p);
    (*func) << s;
    (*func)(0);
}

void CBaseAttInstall::LuaHook(lua_State *L, lua_Debug *ar)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "installer");
    CBaseAttInstall *pInstaller = static_cast<CBaseAttInstall *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
        
    try
    {
        pInstaller->Update();
    }
    catch(Exceptions::CException &e)
    {
        ConvertExToLuaError();
    }
}

int CBaseAttInstall::LuaYesNoBox(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    std::string msg = luaL_checkstring(L, 1);
    int args = lua_gettop(L);
    
    for (int i=2; i<=args; i++)
        msg += luaL_checkstring(L, i);
    
    EscapeControls(msg);
    
    lua_pushboolean(L, pInstaller->YesNoBox(GetTranslation(msg.c_str())));
    
    return 1;
}

int CBaseAttInstall::LuaChoiceBox(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    std::string msg = luaL_checkstring(L, 1);
    const char *but1 = luaL_checkstring(L, 2);
    const char *but2 = luaL_checkstring(L, 3);
    const char *but3 = lua_tostring(L, 4);
    
    EscapeControls(msg);
    
    if (but1)
        but1 = GetTranslation(but1);
    if (but2)
        but2 = GetTranslation(but2);
    if (but3)
        but3 = GetTranslation(but3);

    int ret = pInstaller->ChoiceBox(GetTranslation(msg.c_str()), but1, but2, but3) + 1; // +1: Convert 0-2 range to 1-3
    
    // UNDONE: FLTK doesn't return alternative number while ncurses returns -1.
    // For now we'll stick with the FLTK behaviour: return the last button.
    if (ret == 0) // Ncurses returns -1 and we added 1 thus zero...
        ret = 3;
    
    lua_pushinteger(L, ret);
    
    return 1;
}

int CBaseAttInstall::LuaWarnBox(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    std::string msg = luaL_checkstring(L, 1);
    int args = lua_gettop(L);
    
    for (int i=2; i<=args; i++)
        msg += luaL_checkstring(L, i);
    
    EscapeControls(msg);
    
    pInstaller->WarnBox(GetTranslation(msg.c_str()));
    
    return 0;
}

int CBaseAttInstall::LuaMSGBox(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    std::string msg = luaL_checkstring(L, 1);
    int args = lua_gettop(L);
    
    for (int i=2; i<=args; i++)
        msg += luaL_checkstring(L, i);
    
    EscapeControls(msg);
    
    pInstaller->MsgBox(GetTranslation(msg.c_str()));
    
    return 0;
}

int CBaseAttInstall::LuaNewScreen(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    const char *name = luaL_optstring(L, 1, "");
    NLua::CreateClass(pInstaller->CreateScreen(name), "screen");
    return 1;
}

int CBaseAttInstall::LuaAddScreen(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    CBaseScreen *screen = NLua::CheckClassData<CBaseScreen>("screen", 1);
    pInstaller->AddScreen(screen);
    lua_pop(NLua::LuaState, 1);
    return 0;
}

int CBaseAttInstall::LuaNewProgressDialog(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
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

int CBaseAttInstall::LuaShowDepScreen(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    luaL_checktype(L, 1, LUA_TFUNCTION);
    int ref = NLua::MakeReference(1);
    
    CPointerWrapper<CBaseLuaDepScreen> screen(pInstaller->CoreCreateDepScreen(ref));
    screen->Run();
    NLua::Unreference(ref);

    return 0;
}

int CBaseAttInstall::LuaExecuteCMDAsRoot(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
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

int CBaseAttInstall::LuaAskRootPW(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    pInstaller->GetSUPasswd("This installation requires root (administrator) privileges in order to continue.\n"
            "Please enter the administrative password below.", true);
    return 0;
}

int CBaseAttInstall::LuaLockScreen(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    pInstaller->LockScreen(NLua::LuaToBool(1), NLua::LuaToBool(2), NLua::LuaToBool(3));
    return 0;
}

int CBaseAttInstall::LuaVerifyDestDir(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    lua_pushboolean(L, pInstaller->VerifyDestDir());
    return 1;
}

int CBaseAttInstall::LuaSetAskQuit(lua_State *L)
{
    CBaseAttInstall *pInstaller = NLua::GetFromClosure<CBaseAttInstall *>();
    luaL_checktype(L, 1, LUA_TFUNCTION);
    
    if (pInstaller->m_iAskQuitLuaFunc != LUA_NOREF)
    {
        lua_rawgeti(NLua::LuaState, LUA_REGISTRYINDEX, pInstaller->m_iAskQuitLuaFunc);
        NLua::Unreference(pInstaller->m_iAskQuitLuaFunc);
    }
    
    pInstaller->m_iAskQuitLuaFunc = NLua::MakeReference(1);
    return 1;
}
