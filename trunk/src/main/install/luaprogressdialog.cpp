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

void CBaseLuaProgressDialog::Run()
{
    NLua::CLuaFunc func(m_iFunctionRef, LUA_REGISTRYINDEX);
    if (func)
    {
        NLua::PushClass("progressdialog", this);
        func.PushData(); // Add 'self' to function argument list
        func(0);
    }
}

void CBaseLuaProgressDialog::LuaRegister()
{
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaSetTitle, "settitle", "progressdialog");
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaSetProgress, "setprogress", "progressdialog");
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaEnableSecProgBar, "enablesecbar", "progressdialog");
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaSetSecTitle, "setsectitle", "progressdialog");
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaSetSecProgress, "setsecprogress", "progressdialog");
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaCancelled, "cancelled", "progressdialog");
    NLua::RegisterClassFunction(CBaseLuaProgressDialog::LuaSetCancelButton, "setcancelbutton", "progressdialog");
}

int CBaseLuaProgressDialog::LuaSetTitle(lua_State *L)
{
    CBaseLuaProgressDialog *dialog = NLua::CheckClassData<CBaseLuaProgressDialog>("progressdialog", 1);
    dialog->CoreSetTitle(luaL_checkstring(L, 2));
    return 0;
}

int CBaseLuaProgressDialog::LuaSetProgress(lua_State *L)
{
    CBaseLuaProgressDialog *dialog = NLua::CheckClassData<CBaseLuaProgressDialog>("progressdialog", 1);
    int p = luaL_checkint(L, 2);
    luaL_argcheck(L, ((p >= 0) && (p <= 100)), 2, "Wrong percentage given, should be 0-100.");
    dialog->CoreSetProgress(p);
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

int CBaseLuaProgressDialog::LuaCancelled(lua_State *L)
{
    CBaseLuaProgressDialog *dialog = NLua::CheckClassData<CBaseLuaProgressDialog>("progressdialog", 1);
    lua_pushboolean(L, dialog->m_bCancelled);
    return 1;
}

int CBaseLuaProgressDialog::LuaSetCancelButton(lua_State *L)
{
    CBaseLuaProgressDialog *dialog = NLua::CheckClassData<CBaseLuaProgressDialog>("progressdialog", 1);
    dialog->CoreSetCancelButton(NLua::LuaToBool(2));
    return 0;
}
