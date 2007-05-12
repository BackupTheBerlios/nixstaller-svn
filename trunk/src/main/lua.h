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

#ifndef LUA_H
#define LUA_H

// #include "lua.hpp"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

class CLuaVM
{
    lua_State *m_pLuaState;
    int m_iPushedArgs;

    void GetGlobal(const char *var, const char *tab);
    static int DoFunctionCall(lua_State *L);
    void GetClassMT(const char *type);
    
public:
    CLuaVM(void) : m_pLuaState(NULL), m_iPushedArgs(0) { };
    ~CLuaVM(void) { if (m_pLuaState) lua_close(m_pLuaState); };

    void StackDump(const char *msg);
    
    void Init(void);
    void LoadFile(const char *name);

    void RegisterFunction(lua_CFunction f, const char *name, const char *tab=NULL, void *data=NULL);
    void RegisterNumber(lua_Number n, const char *name, const char *tab=NULL);
    void RegisterString(const char *s, const char *name, const char *tab=NULL);
    template <typename C> void RegisterUData(C *val, const char *type, const char *name, const char *tab=NULL)
    {
        if (!tab)
        {
            CreateClass<C>(val, type);
            lua_setglobal(m_pLuaState, name);
        }
        else
        {
            lua_getglobal(m_pLuaState, tab);
    
            if (lua_isnil(m_pLuaState, -1))
            {
                lua_pop(m_pLuaState, 1);
                lua_newtable(m_pLuaState);
            }
    
            CreateClass<C>(val, type);
            lua_setfield(m_pLuaState, -2, name);
    
            lua_setglobal(m_pLuaState, tab);
        }
    }

    bool InitCall(const char *func, const char *tab=NULL);
    template <typename C> bool InitCall(const char *func, const char *type, C *prvdata)
    {
        if (luaL_getmetafield(m_pLuaState, LUA_REGISTRYINDEX, type))
        {
            int mt = lua_gettop(m_pLuaState);
            
            // get mt[prvdata]
            lua_pushlightuserdata(m_pLuaState, reinterpret_cast<void *>(prvdata));
            lua_gettable(m_pLuaState, mt);

            if (!lua_isnil(m_pLuaState, -1))
            {
                lua_getfield(m_pLuaState, -1, func);
                lua_remove(m_pLuaState, -2); // Remove class-table
            }
            lua_remove(m_pLuaState, mt);
            return !lua_isnil(m_pLuaState, -1);
        }
        
        return true;
    }
    void PushArg(lua_Number n);
    void PushArg(const char *s);
    void DoCall(void);

    bool GetNumVar(lua_Number *out, const char *var, const char *tab=NULL);
    bool GetNumVar(lua_Integer *out, const char *var, const char *tab=NULL);
    bool GetStrVar(std::string *out, const char *var, const char *tab=NULL);
    const char *GetStrVar(const char *var, const char *tab=NULL);

    unsigned OpenArray(const char *var, const char *tab=NULL);
    bool GetArrayNum(unsigned &index, lua_Number *out);
    bool GetArrayNum(unsigned &index, lua_Integer *out);
    bool GetArrayStr(unsigned &index, std::string *out);
    bool GetArrayStr(unsigned &index, char *out);
    template <typename C> bool GetArrayClass(unsigned &index, C *&out, const char *type)
    {
        return false; // UNDONE
        lua_rawgeti(m_pLuaState, -1, index);

        C *val = ToClassData<C>(type, -1);
        lua_pop(m_pLuaState, -1);

        if (val)
        {
            out = val;
            return true;
        }
    
        return false;
    }
    void CloseArray(void);

    void SetClassGC(const char *name, lua_CFunction gc, void *gcdata=NULL);
    template <typename C> void CreateClass(C *prvdata, const char *type)
    {
        lua_newtable(m_pLuaState);
        int table = lua_gettop(m_pLuaState);
    
        GetClassMT(type);
        int mt = lua_gettop(m_pLuaState);

        // mt[prvdata] = table
        lua_pushlightuserdata(m_pLuaState, reinterpret_cast<void *>(prvdata));
        lua_pushvalue(m_pLuaState, table);
        lua_settable(m_pLuaState, mt);

        // mt[table] = prvdata
        lua_pushvalue(m_pLuaState, table);
        lua_pushlightuserdata(m_pLuaState, reinterpret_cast<void *>(prvdata));
        lua_settable(m_pLuaState, mt);

        lua_setmetatable(m_pLuaState, table);
    }
    void RegisterClassFunc(const char *type, lua_CFunction f, const char *name, void *data=NULL);
    template <typename C> C *CheckClassData(const char *type, int index)
    {
        luaL_checktype(m_pLuaState, index, LUA_TTABLE);
        C *ret = ToClassData<C>(type, index);
        
        if (!ret)
            luaL_typerror(m_pLuaState, index, type);
        
        return ret;
    }
    template <typename C> C *ToClassData(const char *type, int index)
    {
        if (index < 0) // Relative index given, convert
            index = (lua_gettop(m_pLuaState)+1) + index;
        
        C *ret = NULL;
        if (lua_getmetatable(m_pLuaState, index))
        {
            int mt = lua_gettop(m_pLuaState);
            lua_getfield(m_pLuaState, LUA_REGISTRYINDEX, type);
            if (lua_rawequal(m_pLuaState, -1, -2))
            {
                // get mt[table]
                lua_pushvalue(m_pLuaState, index);
                lua_gettable(m_pLuaState, mt);
                
                if (!lua_isnil(m_pLuaState, -1))
                    ret = reinterpret_cast<C *>(lua_touserdata(m_pLuaState, -1));
                lua_pop(m_pLuaState, 1);
            }
            lua_pop(m_pLuaState, 1);
        }
        lua_pop(m_pLuaState, 1);
        
        return ret;
    }
    
    void SetArrayStr(const char *s, const char *var, int index, const char *tab=NULL);

    bool GetArgNum(lua_Number *out);
    bool GetArgNum(lua_Integer *out);
    bool GetArgStr(std::string *out);
    bool GetArgStr(char *out);

    void LuaError(const char *msg, ...);
};

#endif
