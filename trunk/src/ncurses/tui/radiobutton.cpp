/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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

#include <algorithm>
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
            Select(0); // CoreSelect (called by Select) sets m_bInitSelect to false
    }
}

std::string CRadioButton::CoreGetText(const SEntry &entry)
{
    const char *base = (entry.enabled) ? "(X) " : "( ) ";
    return base + entry.name;
}

void CRadioButton::CoreSelect(SEntry &entry)
{
    if (m_pActiveEntry)
        m_pActiveEntry->enabled = false;
    entry.enabled = true;
    m_pActiveEntry = &entry;
    m_bInitSelect = false;
}

void CRadioButton::CoreDeleteChoice(SEntry &entry)
{
    if (&entry == m_pActiveEntry)
    {
        TChoiceList &list = GetChoiceList();
        TChoiceList::iterator it = std::find(list.begin(), list.end(), entry);
        if (it != list.end())
        {
            if (it != list.begin())
                m_pActiveEntry = &*(it-1);
            else if (it != list.end()-1)
                m_pActiveEntry = &*(it+1);
            else
                m_pActiveEntry = NULL;
        }
        else
            m_pActiveEntry = NULL;
        
        if (m_pActiveEntry)
            m_pActiveEntry->enabled = true;
    }
}

void CRadioButton::CoreGetButtonDescs(TButtonDescList &list)
{
    CBaseChoice::CoreGetButtonDescs(list);
    list.push_back(TButtonDescPair("SPACE", "Enable"));
}

}
