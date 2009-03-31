/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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

#include "main/lua/lua.h"
#include "main/lua/luatable.h"
#include "main/frontend/unattinstall.h"

// -------------------------------------
// Base Attended Installer Class
// -------------------------------------

void CBaseUnattInstall::InitLua()
{
    CBaseInstall::InitLua();
    
    NLua::LuaSet(true, "unattended", "install");

    RunLua("install.lua");
}

void CBaseUnattInstall::Init(int argc, char **argv)
{
    NLua::CLuaTable tab("args");
    bool start = false;
    int tabind = 1;
    for (int i=1; i<argc; i++)
    {
        if (!start)
            start = (!strcmp(argv[i], "--"));
        else
        {
            tab[tabind] << argv[i];
            tabind++;
        }
    }
    tab.Close();
    
    CBaseInstall::Init(argc, argv);
}
