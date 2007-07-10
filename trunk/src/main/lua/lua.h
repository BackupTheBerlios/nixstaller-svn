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

#ifndef LUA_WRAPPER_H
#define LUA_WRAPPER_H

#include <string>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

namespace NLua {

class CLuaStateKeeper
{
    lua_State *m_pLuaState;
    
public:
    CLuaStateKeeper(void);
    ~CLuaStateKeeper(void) { if (m_pLuaState) lua_close(m_pLuaState); }
    
    operator lua_State*(void) { return m_pLuaState; }
};

void StackDump(const char *msg);

void GetGlobal(const char *var, const char *tab);
void LoadFile(const char *name);

bool LuaGet(std::string &out, const char *var, const char *tab=NULL);
bool LuaGet(std::string &out, const char *var, const char *type, void *prvdata);
bool LuaGet(int &out, const char *var, const char *tab=NULL);
bool LuaGet(int &out, const char *var, const char *type, void *prvdata);

void LuaSet(const std::string &val, const char *var, const char *tab=NULL);
void LuaSet(const std::string &val, const char *var, const char *type, void *prvdata);
void LuaSet(int val, const char *var, const char *tab=NULL);
void LuaSet(int val, const char *var, const char *type, void *prvdata);

int MakeReference(int index, int tab=LUA_REGISTRYINDEX);
void Unreference(int ref, int tab=LUA_REGISTRYINDEX);

bool LuaToBool(int index);

extern CLuaStateKeeper LuaState;

}

#endif
