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
#include "luaradiobutton.h"
#include "tui/radiobutton.h"

// -------------------------------------
// Lua Radio Button Class
// -------------------------------------

CLuaRadioButton::CLuaRadioButton(const char *desc, const TOptions &l) : CLuaWidget(desc), m_Options(l)
{
    m_pRadioButton = new NNCurses::CRadioButton;
    
    for (TOptions::const_iterator it=l.begin(); it!=l.end(); it++)
        m_pRadioButton->AddChoice(GetTranslation(*it));
    
    AddWidget(m_pRadioButton);
}

const char *CLuaRadioButton::EnabledButton()
{
    for (TOptions::iterator it=m_Options.begin(); it!=m_Options.end(); it++)
    {
        if (m_pRadioButton->GetSelection() == GetTranslation(*it))
            return it->c_str();
    }
    
    return NULL;
}

void CLuaRadioButton::Enable(int n)
{
    m_pRadioButton->Select(n);
}

void CLuaRadioButton::CoreUpdateLanguage()
{
    int n = 1;
    for (TOptions::iterator it=m_Options.begin(); it!=m_Options.end(); it++, n++)
        m_pRadioButton->SetName(n, GetTranslation(*it));
}

bool CLuaRadioButton::CoreHandleEvent(NNCurses::CWidget *emitter, int event)
{
    if ((emitter == m_pRadioButton) && (event == EVENT_DATACHANGED))
    {
        LuaDataChanged();
        return true;
    }
    
    return false;
}
