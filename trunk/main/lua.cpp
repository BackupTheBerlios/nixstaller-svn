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
    m_pLuaVM = lua_open();
    
    if (!m_pLuaVM)
        return false;
        
    const luaL_Reg *lib = libtable;
    for (; lib->func; lib++)
    {
        lua_pushcfunction(m_pLuaVM, lib->func);
        lua_pushstring(m_pLuaVM, lib->name);
        lua_call(m_pLuaVM, 1, 0);
        lua_settop(m_pLuaVM, 0);  // Clear stack
    }
    
    return true;
}

void CLuaVM::RegisterTable(const char *name, luaL_Reg *p)
{
	luaL_register(m_pLuaVM, name, p);
}

void RegisterVar(const char *name, const char *val, const char *env)
{
	if (env)
		luaL_register(m_pLuaVM, 