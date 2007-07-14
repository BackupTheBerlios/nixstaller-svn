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
#include "luamenu.h"
#include "tui/menu.h"

// -------------------------------------
// Lua Menu Class
// -------------------------------------

CLuaMenu::CLuaMenu(const char *desc, const TOptions &l) : CLuaWidget(desc), m_Options(l)
{
    m_pMenu = new NNCurses::CMenu(15, 7);
    
    for (TOptions::const_iterator it=l.begin(); it!=l.end(); it++)
        m_pMenu->AddEntry(*it, GetTranslation(*it));
    
    AddWidget(m_pMenu);
}

const char *CLuaMenu::Selection()
{
    if (m_pMenu->Empty())
        return NULL;
    
    for (TOptions::iterator it=m_Options.begin(); it!=m_Options.end(); it++)
    {
        if (GetTranslation(*it) == m_pMenu->Value())
            return it->c_str();
    }
    
    return NULL;
}

void CLuaMenu::Select(int n)
{
    m_pMenu->Select(m_Options.at(n-1));
}

void CLuaMenu::CoreUpdateLanguage()
{
    int n = 1;
    for (TOptions::iterator it=m_Options.begin(); it!=m_Options.end(); it++, n++)
        m_pMenu->SetName(*it, GetTranslation(*it));
}
