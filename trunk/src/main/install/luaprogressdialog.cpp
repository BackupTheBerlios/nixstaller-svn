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

#include "main/main.h"
#include "luaprogressdialog.h"
#include "main/lua/lua.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"

// -------------------------------------
// Base Lua Progress Dialog Class
// -------------------------------------

void CBaseLuaProgressDialog::CallFunction()
{
    NLua::CLuaFunc func(m_iFunctionRef, LUA_REGISTRYINDEX);
    if (func)
    {
        NLua::PushClass("progressdialog", this);
        func.PushData(); // Add 'self' to function argument list
        func(1);
    }
}

void CBaseLuaProgressDialog::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaNextStep, "nextstep", "progressdialog");
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaEnableSecProgBar, "enablesecbar", "progressdialog");
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaSetSecTitle, "setsectitle", "progressdialog");
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaSetSecProgress, "setsecprogress", "progressdialog");
}

int CBaseLuaProgressDialog::LuaNextStep(lua_State *L)
{
    CBaseLuaProgressDialog *dialog = NLua::CheckClassData<CBaseLuaProgressDialog>("progressdialog", 1);
    if (dialog->m_CurrentStep < (dialog->m_StepList.size() - 1))
    {
        dialog->m_CurrentStep++;
        dialog->CoreSetNextStep();
    }
    return 0;
}

int CBaseLuaProgressDialog::LuaEnableSecProgBar(lua_State *L)
{
    CBaseLuaProgressDialog *dialog = NLua::CheckClassData<CBaseLuaProgressDialog>("progressdialog", 1);
    dialog->CoreEnableSecProgBar(NLua::LuaToBool(2));
    return 0;
}

int CBaseLuaProgressDialog::LuaSetSecTitle(lua_State *L)
{
    CBaseLuaProgressDialog *dialog = NLua::CheckClassData<CBaseLuaProgressDialog>("progressdialog", 1);
    dialog->CoreSetSecTitle(luaL_checkstring(L, 2));
    return 0;
}

int CBaseLuaProgressDialog::LuaSetSecProgress(lua_State *L)
{
    CBaseLuaProgressDialog *dialog = NLua::CheckClassData<CBaseLuaProgressDialog>("progressdialog", 1);
    dialog->CoreSetSecProgress(luaL_checkint(L, 2));
    return 0;
}
