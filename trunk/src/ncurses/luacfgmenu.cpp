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

#include "luacfgmenu.h"
#include "tui/menu.h"

// -------------------------------------
// Lua Config Menu Class
// -------------------------------------

CLuaCFGMenu::CLuaCFGMenu(const char *desc) : CLuaWidget(desc)
{
    AddWidget(m_pMenu = new NNCurses::CMenu(25, 10));
}

void CLuaCFGMenu::CoreAddVar(const char *name)
{
    m_pMenu->AddEntry(name, GetTranslation(name));
}

void CLuaCFGMenu::CoreUpdateLanguage()
{
    for (TVarType::iterator it=GetVariables().begin(); it!=GetVariables().end(); it++)
        m_pMenu->SetName(it->first, GetTranslation(it->first));
}
