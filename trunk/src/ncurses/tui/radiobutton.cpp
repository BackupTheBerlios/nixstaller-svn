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

#include "tui.h"
#include "radiobutton.h"

namespace NNCurses {

// -------------------------------------
// Radio Button Class
// -------------------------------------

void CRadioButton::CoreInit()
{
    if (m_bInitSelect)
    {
        TChoiceList &list = GetChoiceList();
        if (!list.empty())
            Select(1); // CoreSelect (called by Select) sets m_bInitSelect to false
    }
}

std::string CRadioButton::CoreGetText(const SEntry &entry)
{
    const char *base = (entry.enabled) ? "(X) " : "( ) ";
    return base + entry.name;
}

void CRadioButton::CoreSelect(SEntry &entry)
{
    TChoiceList &list = GetChoiceList();
    
    list.at(m_ActiveEntry).enabled = false;
    entry.enabled = true;
    m_ActiveEntry = std::distance(list.begin(), std::find(list.begin(), list.end(), entry));
    m_bInitSelect = false;
}

void CRadioButton::CoreGetButtonDescs(TButtonDescList &list)
{
    CBaseChoice::CoreGetButtonDescs(list);
    list.push_back(TButtonDescPair("SPACE", "Enable"));
}

}
