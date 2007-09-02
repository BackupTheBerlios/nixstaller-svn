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
    int x = 0, y = (Height() / SafeConvert<int>(m_ChoiceList.size())) / 2, cur = 0;
    for (TChoiceList::iterator it=m_ChoiceList.begin(); it!=m_ChoiceList.end(); it++, cur++)
    {
        std::string text = CoreGetText(*it);
        int length = SafeConvert<int>(text.length());
        
        if ((x + length) > m_iUsedWidth)
        {
            x = 0;
            y++;
        }
        
        bool highlight = (cur == m_iSelection);
        
        if (highlight)
            SetAttr(this, A_REVERSE, true);
        
        AddStr(this, x, y, text.c_str());
        
        if (highlight)
            SetAttr(this, A_REVERSE, false);
        
        x += length + 1;
    }
}

int CBaseChoice::CoreRequestWidth()
{
    int ret = 1;
    
    for (TChoiceList::iterator it=m_ChoiceList.begin(); it!=m_ChoiceList.end(); it++)
        ret = std::max(ret, SafeConvert<int>(CoreGetText(*it).length()));
    
    if (GetMinWidth())
        ret = std::max(ret, GetMinWidth());

    m_iUsedWidth = ret; // May get more, but for keeping layout code easy we just stick with this width
    return ret;
}

int CBaseChoice::CoreRequestHeight()
{
    int width = RequestWidth(), lines = 1, x = 0;
    
    if (!m_ChoiceList.empty())
    {
        for (TChoiceList::iterator it=m_ChoiceList.begin(); it!=m_ChoiceList.end(); it++)
        {
            int length = SafeConvert<int>(CoreGetText(*it).length());
            
            if ((x + length) > width)
            {
                x = 0;
                lines++;
            }

            x += CoreGetText(*it).length() + 1;
        }
    }
    
    if (GetMinHeight())
        lines = std::max(lines, GetMinHeight());

    return lines;
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
        Select(m_iSelection);
        PushEvent(EVENT_DATACHANGED);
        RequestQueuedDraw();
    }
    else
        return false;
    
    return true;
}

void CBaseChoice::CoreGetButtonDescs(TButtonDescList &list)
{
    list.push_back(TButtonDescPair("ARROWS", "Navigate"));
}


}
