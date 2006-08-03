/*
    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.

    Nixstaller is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Nixstaller is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nixstaller; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
    conditions of the GNU General Public License cover the whole combination.

    In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
    software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
    DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
    such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
    you include the source code of that other code when and as the GNU GPL requires distribution of source code.

    Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
    versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
    version without this exception; this exception also makes it possible to release a modified version which carries forward
    this exception.
*/

#include "main.h"

// Table of libs that should be loaded
static const luaL_Reg libtable[] = {
    { "", luaopen_base },
    { LUA_TABLIBNAME, luaopen_table },
    { LUA_IOLIBNAME, luaopen_io },
    { LUA_OSLIBNAME, luaopen_os },
    { LUA_STRLIBNAME, luaopen_string },
    { LUA_MATHLIBNAME, luaopen_math },
 // { LUA_LOADLIBNAME, luaopen_package }, // Perhaps later
 // { LUA_DBLIBNAME, luaopen_debug }, // No debug
    { NULL, NULL }
};
    
// -------------------------------------
// Lua Class
// -------------------------------------

bool CLuaVM::Init()
{
    // Initialize lua
    m_pLuaState = lua_open();
    
    if (!m_pLuaState)
        return false;
        
    const luaL_Reg *lib = libtable;
    for (; lib->func; lib++)
    {
        lua_pushcfunction(m_pLuaState, lib->func);
        lua_pushstring(m_pLuaState, lib->name);
        lua_call(m_pLuaState, 1, 0);
        lua_settop(m_pLuaState, 0);  // Clear stack
    }
    
    return true;
}

bool CLuaVM::LoadFile(const char *name)
{
    if (luaL_dofile(m_pLuaState, name))
    {
        const char *errmsg = lua_tostring(m_pLuaState, -1);
        if (!errmsg)
            errmsg = "Unknown error!";
        //ThrowError(false, "While parsing %s: %s", name, errmsg);
        fprintf(stderr, "While parsing %s: %s", name, errmsg);
        return false;
    }
    return true;
}

void CLuaVM::RegisterFunction(lua_CFunction f, const char *name, const char *tab)
{
    if (!tab)
        lua_register(m_pLuaState, name, f);
    else
    {
        lua_getglobal(m_pLuaState, tab);
        
        bool neednewtab = lua_isnil(m_pLuaState, -1);
        
        if (neednewtab)
        {
            lua_pop(m_pLuaState, 1);
            lua_newtable(m_pLuaState);
        }
        
        lua_pushstring(m_pLuaState, name);
        lua_pushcfunction(m_pLuaState, f);
        lua_settable(m_pLuaState, -3);
        
        //if (neednewtab)
            lua_setglobal(m_pLuaState, tab);
        //else
          //  lua_pop
    }
}

void CLuaVM::RegisterNumber(lua_Number n, const char *name, const char *tab)
{
    if (!tab)
    {
        lua_pushnumber(m_pLuaState, n);
        lua_setglobal(m_pLuaState, name);
    }
    else
    {
        lua_getglobal(m_pLuaState, tab);
        
        bool neednewtab = lua_isnil(m_pLuaState, -1);
        
        if (neednewtab)
        {
            lua_pop(m_pLuaState, 1);
            lua_newtable(m_pLuaState);
        }
        
        lua_pushstring(m_pLuaState, name);
        lua_pushnumber(m_pLuaState, n);
        lua_settable(m_pLuaState, -3);
        
        //if (neednewtab)
            lua_setglobal(m_pLuaState, tab);
        //else
          //  lua_pop
    }
}
