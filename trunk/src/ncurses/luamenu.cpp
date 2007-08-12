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

CLuaMenu::CLuaMenu(const char *desc, const TOptions &l) : CBaseLuaMenu(l), CLuaWidget(desc)
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
    
    TOptions &opts = GetOptions();
    
    for (TOptions::iterator it=opts.begin(); it!=opts.end(); it++)
    {
        if (*it == m_pMenu->Value())
            return it->c_str();
    }
    
    return NULL;
}

void CLuaMenu::Select(TSTLVecSize n)
{
    m_pMenu->Select(GetOptions().at(n));
}

void CLuaMenu::CoreUpdateLanguage()
{
    CLuaWidget::CoreUpdateLanguage();

    TOptions &opts = GetOptions();
    for (TOptions::iterator it=opts.begin(); it!=opts.end(); it++)
        m_pMenu->SetName(*it, GetTranslation(*it));
}

bool CLuaMenu::CoreHandleEvent(NNCurses::CWidget *emitter, int event)
{
    if ((emitter == m_pMenu) && (event == EVENT_DATACHANGED))
    {
        LuaDataChanged();
        return true;
    }
    
    return false;
}
