/*
    Copyright (C) 2007 Rick Helmus (rhelmus_AT_gmail.com)

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

#include "lua.h"
#include "luaclass.h"
#include "../main.h"

namespace {

// Table of libs that should be loaded
const luaL_Reg libtable[] = {
    { "", luaopen_base },
    { LUA_TABLIBNAME, luaopen_table },
    { LUA_IOLIBNAME, luaopen_io },
    { LUA_OSLIBNAME, luaopen_os },
    { LUA_STRLIBNAME, luaopen_string },
    { LUA_MATHLIBNAME, luaopen_math },
    { LUA_LOADLIBNAME, luaopen_package },
//   { LUA_DBLIBNAME, luaopen_debug }, // No debug
    { NULL, NULL }
};

bool PushGlobalVar(const char *var, const char *tab=NULL)
{
    NLua::GetGlobal(var, tab);
    
    if (lua_isnil(NLua::LuaState, -1))
    {
        lua_pop(NLua::LuaState, 1);
        return false;
    }
    
    return true;
}

bool PushClassVar(const char *var, const char *type, void *prvdata)
{
    NLua::PushClass(type, prvdata);
    
    if (!lua_isnil(NLua::LuaState, -1))
    {
        lua_getfield(NLua::LuaState, -1, var);
        lua_remove(NLua::LuaState, -2); // Remove class-table
    }
    
    if (lua_isnil(NLua::LuaState, -1))
    {
        lua_pop(NLua::LuaState, 1);
        return false;
    }
    
    return true;
}

// Sets a value from the stack
void SetGlobalVar(const char *var, const char *tab)
{
    if (!tab)
        lua_setglobal(NLua::LuaState, var);
    else
    {
        lua_getglobal(NLua::LuaState, tab);
        
        if (lua_isnil(NLua::LuaState, -1))
        {
            lua_pop(NLua::LuaState, 1);
            lua_newtable(NLua::LuaState);
        }
        
        lua_insert(NLua::LuaState, -2); // Swap table <--> value
        lua_setfield(NLua::LuaState, -2, var);
        
        lua_setglobal(NLua::LuaState, tab);
    }
}

void SetClassVar(const char *var, const char *type, void *prvdata)
{
    NLua::PushClass(type, prvdata);
    lua_insert(NLua::LuaState, -2); // Swap class <--> value
    lua_setfield(NLua::LuaState, -2, var);
    lua_pop(NLua::LuaState, 1); // Remove class-table
}


}


namespace NLua {

CLuaStateKeeper LuaState;


// -------------------------------------
// Lua State Swrapper Class
// -------------------------------------

CLuaStateKeeper::CLuaStateKeeper()
{
    // Initialize lua
    m_pLuaState = lua_open();
    
    if (!LuaState)
        throw Exceptions::CExLua("Could not open lua VM");
        
    const luaL_Reg *lib = libtable;
    for (; lib->func; lib++)
    {
        lua_pushcfunction(LuaState, lib->func);
        lua_pushstring(LuaState, lib->name);
        lua_call(LuaState, 1, 0);
        lua_settop(LuaState, 0);  // Clear stack
    }
}


// -------------------------------------
// Utilities
// -------------------------------------

void GetGlobal(const char *var, const char *tab)
{
    if (tab)
    {
        lua_getglobal(LuaState, tab);
        if (!lua_isnil(LuaState, -1))
        {
            lua_getfield(LuaState, -1, var);
            lua_remove(LuaState, -2); // Remove table from stack(we don't need it anymore)
        }
    }
    else
        lua_getglobal(LuaState, var);
}

void LoadFile(const char *name)
{
    if (luaL_dofile(LuaState, name))
    {
        const char *errmsg = lua_tostring(LuaState, -1);
        if (!errmsg)
            errmsg = "Unknown error!";
        else if (!strcmp(errmsg, "CExUser"))
            throw Exceptions::CExUser();
            
        throw Exceptions::CExLua(CreateText("While parsing %s: %s\n", name, errmsg));
    }
}

bool LuaGet(std::string &out, const char *var, const char *tab)
{
    if (!PushGlobalVar(var, tab))
        return false;
    
    bool goodtype = lua_isstring(LuaState, -1);
    if (goodtype)
        out = lua_tostring(LuaState, -1);
    
    lua_pop(LuaState, 1);
    
    return (goodtype);
}

bool LuaGet(std::string &out, const char *var, const char *type, void *prvdata)
{
    if (!PushClassVar(var, type, prvdata))
        return false;
    
    bool goodtype = lua_isstring(LuaState, -1);
    if (goodtype)
        out = lua_tostring(LuaState, -1);
    
    lua_pop(LuaState, 1);
    
    return (goodtype);
}

bool LuaGet(int &out, const char *var, const char *tab)
{
    if (!PushGlobalVar(var, tab))
        return false;
    
    bool goodtype = lua_isnumber(LuaState, -1);
    if (goodtype)
        out = lua_tointeger(LuaState, -1);
    
    lua_pop(LuaState, 1);
    
    return (goodtype);
}

bool LuaGet(int &out, const char *var, const char *type, void *prvdata)
{
    if (!PushClassVar(var, type, prvdata))
        return false;
    
    bool goodtype = lua_isnumber(LuaState, -1);
    if (goodtype)
        out = lua_tointeger(LuaState, -1);
    
    lua_pop(LuaState, 1);
    
    return (goodtype);
}

void LuaSet(const std::string &val, const char *var, const char *tab)
{
    lua_pushstring(LuaState, val.c_str());
    SetGlobalVar(var, tab);
}

void LuaSet(const std::string &val, const char *var, const char *type, void *prvdata)
{
    lua_pushstring(LuaState, val.c_str());
    SetClassVar(var, type, prvdata);
}

void LuaSet(int val, const char *var, const char *tab)
{
    lua_pushinteger(LuaState, val);
    SetGlobalVar(var, tab);
}

void LuaSet(int val, const char *var, const char *type, void *prvdata)
{
    lua_pushinteger(LuaState, val);
    SetClassVar(var, type, prvdata);
}


}
