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
#include "luacheckbox.h"
#include "tui/checkbox.h"

// -------------------------------------
// Lua Checkbox Class
// -------------------------------------

CLuaCheckbox::CLuaCheckbox(const char *desc, const TOptions &l) : CBaseLuaCheckbox(l), CLuaWidget(desc)
{
    m_pCheckbox = new NNCurses::CCheckbox;
    
    for (TOptions::const_iterator it=l.begin(); it!=l.end(); it++)
        m_pCheckbox->AddChoice(GetTranslation(*it));
    
    AddWidget(m_pCheckbox);
}

bool CLuaCheckbox::Enabled(TSTLVecSize n)
{
    NNCurses::CCheckbox::TRetType l;
    m_pCheckbox->GetSelections(l);
    
    if (std::find(l.begin(), l.end(), GetTranslation(GetOptions().at(n))) != l.end())
        return true;
    
    return false;
}

void CLuaCheckbox::Enable(TSTLVecSize n, bool b)
{
    if (Enabled(n) != b)
        m_pCheckbox->Select(n);
}

void CLuaCheckbox::CoreUpdateLanguage()
{
    TOptions &opts = GetOptions();
    int n = 0;
    
    for (TOptions::iterator it=opts.begin(); it!=opts.end(); it++, n++)
        m_pCheckbox->SetName(n, GetTranslation(*it));
}

bool CLuaCheckbox::CoreHandleEvent(NNCurses::CWidget *emitter, int event)
{
    if ((emitter == m_pCheckbox) && (event == EVENT_DATACHANGED))
    {
        LuaDataChanged();
        return true;
    }
    
    return false;
}
