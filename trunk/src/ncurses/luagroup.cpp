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

#include "main/main.h"
#include "luagroup.h"
#include "luacfgmenu.h"
#include "luainput.h"

// -------------------------------------
// Lua Widget Group Class
// -------------------------------------

CBaseLuaInputField *CLuaGroup::CreateInputField(const char *label, const char *desc, const char *val,
                                                int max, const char *type)
{
    CLuaInputField *ret = new CLuaInputField(label, desc, val, max, type);
    AddWidget(ret);
    return ret;
}

CBaseLuaCFGMenu *CLuaGroup::CreateCFGMenu(const char *desc)
{
    CLuaCFGMenu *ret = new CLuaCFGMenu(desc);
    AddWidget(ret);
    return ret;
}

void CLuaGroup::CoreUpdateLanguage()
{
}
