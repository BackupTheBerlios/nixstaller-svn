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

#include "main.h"
#include "tui.h"
#include "basechoice.h"

namespace NNCurses {

// -------------------------------------
// Base Choice Class
// -------------------------------------

void CBaseChoice::Move(int n)
{
    m_iSelection += n;
    
    int size = SafeConvert<int>(m_ChoiceList.size());
    
    if (m_iSelection < 0)
        m_iSelection = size - 1;
    else if (m_iSelection >= size)
        m_iSelection = 0;
    
    RequestQueuedDraw();
}

void CBaseChoice::DoDraw()
{
    int x = 0, y = 0, cur = 0;
    for (TChoiceList::iterator it=m_ChoiceList.begin(); it!=m_ChoiceList.end(); it++, cur++)
    {
        std::string text = CoreGetText(*it);
        int length = SafeConvert<int>(text.length());
        
        if ((x + length) > Width())
        {
            x = 0;
            y++;
        }
        
        bool highlight = (cur == m_iSelection);
        
        if (highlight)
            wattron(GetWin(), A_REVERSE);
        
        mvwaddstr(GetWin(), y, x, CoreGetText(*it).c_str());
        
        if (highlight)
            wattroff(GetWin(), A_REVERSE);
        
        x += length + 1;
    }
}

int CBaseChoice::CoreRequestWidth()
{
    if (GetMinWidth())
        return GetMinWidth();
    
    int ret = 1;
    
    for (TChoiceList::iterator it=m_ChoiceList.begin(); it!=m_ChoiceList.end(); it++)
        ret = std::max(ret, SafeConvert<int>(CoreGetText(*it).length()));
    
    return ret;
}

int CBaseChoice::CoreRequestHeight()
{
    if (GetMinHeight())
        return GetMinHeight();
    
    int width = RequestWidth(), lines = 0, x = 0;
    
    for (TChoiceList::iterator it=m_ChoiceList.begin(); it!=m_ChoiceList.end(); it++)
    {
        x += it->name.length() + 1;
        
        if (x > width)
        {
            x = 0;
            lines++;
        }
    }
    
    return std::max(1, lines);
}

bool CBaseChoice::CoreHandleKey(chtype key)
{
    if (key == KEY_UP)
        Move(-1);
    else if (key == KEY_DOWN)
        Move(1);
    else if (IsEnter(key))
        PushEvent(EVENT_CALLBACK);
    else if (key == ' ')
    {
        Select(m_iSelection+1);
        PushEvent(EVENT_DATACHANGED);
        RequestQueuedDraw();
    }
    else
        return false;
    
    return true;
}


}
