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

#ifndef LUACFGMENU_H
#define LUACFGMENU_H

struct lua_State;

class CBaseLuaCFGMenu
{
protected:
    enum EVarType { TYPE_DIR, TYPE_STRING, TYPE_LIST, TYPE_BOOL };

    typedef std::vector<std::string> TOptionsType;

    struct entry_s
    {
        std::string val, def, desc;
        TOptionsType options;
        EVarType type;
        entry_s(const char *v, const char *d, EVarType t) : val(v), def(v), desc(d), type(t) { };
    };
    
    typedef std::map<std::string, entry_s *> TVarType;

    TVarType m_Variabeles;
    
    const char *BoolToStr(bool b) { return (b) ? "Enable" : "Disable"; }

public:
    virtual ~CBaseLuaCFGMenu(void);
    virtual void AddVar(const char *name, const char *desc, const char *val, EVarType type, TOptionsType *l=NULL);
    
    static void LuaRegister(void);
    
    static int LuaAddDir(lua_State *L);
    static int LuaAddString(lua_State *L);
    static int LuaAddList(lua_State *L);
    static int LuaAddBool(lua_State *L);
    static int LuaGet(lua_State *L);
};


#endif
