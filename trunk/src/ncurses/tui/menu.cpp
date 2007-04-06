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
#include "menu.h"

namespace NNCurses {

void CMenu::Move(int n)
{
    int lines = SafeConvert<int>(m_MenuList.size()), curyoffset = m_iYOffset, curcursor = m_iCursorLine;
    int h = Height()-2;

    if (n < 0)
    {
        int nabs = abs(n);
        if (nabs > m_iCursorLine)
        {
            int rest = (nabs - m_iCursorLine);
            
            if (rest > m_iYOffset)
            {
                m_iYOffset = 0;
                VScroll(0, false);
            }
            else
                m_iYOffset -= rest;
            
            m_iCursorLine = 0;
        }
        else
            m_iCursorLine -= nabs;
    }
    else
    {
        int newpos = m_iCursorLine + n;
        if (((newpos+m_iYOffset) >= lines) || (newpos < m_iCursorLine)) // Don't go past menu size and prevent overflows
            newpos = ((lines-1) - m_iYOffset);
        
        m_iCursorLine = newpos;
    }

    if (m_iCursorLine >= h)
    {
        m_iYOffset += (m_iCursorLine - (h-1));
        
        int max = lines - h;
        if (m_iYOffset > max)
            m_iYOffset = max;
                
        m_iCursorLine = h-1;
    }
    
    if (curyoffset != m_iYOffset)
        VScroll(m_iYOffset, false);
    
    if ((curyoffset != m_iYOffset) || (curcursor != m_iCursorLine))
    {
        RequestQueuedDraw();
        PushEvent(EVENT_DATACHANGED);
    }
}

void CMenu::DoDraw()
{
    int y = 1;
    TMenuList::iterator it=m_MenuList.begin() + m_iYOffset;
    
    for (; ((it!=m_MenuList.end()) && (y<Height()-1)); it++, y++)
    {
        bool highlight = ((y-1) == m_iCursorLine);
        
        if (highlight)
            wattron(GetWin(), A_REVERSE);
        
        int len = SafeConvert<int>(it->name.length());
        if (len >= m_iXOffset)
        {
            int end = std::min(Width(), len-m_iXOffset);
            mvwaddstr(GetWin(), y, 1, it->name.substr(m_iXOffset, end).c_str());
        }
        
        if (highlight)
            wattroff(GetWin(), A_REVERSE);
    }
}

bool CMenu::CoreHandleKey(chtype key)
{
    if (CBaseScroll::CoreHandleKey(key))
        return true;

    switch (key)
    {
        case KEY_UP:
            Move(-1);
            return true;
        case KEY_DOWN:
            Move(1);
            return true;
        case KEY_LEFT:
            HScroll(-1, true);
            return true;
        case KEY_RIGHT:
            HScroll(1, true);
            return true;
        case KEY_PPAGE:
            Move(-ScrollFieldHeight()+1);
            return true;
        case KEY_NPAGE:
            Move(ScrollFieldHeight()-1);
            return true;
        default:
            if (IsEnter(key))
            {
                PushEvent(EVENT_CALLBACK);
                return true;
            }
            else if (isprint(key)) // Go to item which starts with typed character
            {
                // First try from current position
                TMenuList::iterator cur = m_MenuList.begin() + GetCurrent();
                TMenuList::iterator line = std::lower_bound(cur+1, m_MenuList.end(), key);
                
                if ((line == m_MenuList.end()) || (line->name[0] != key)) // Not found, start from begin
                    line = std::lower_bound(m_MenuList.begin(), cur, key);
                
                if ((line != m_MenuList.end()) && (line->name[0] == key))
                {
                    VScroll(SafeConvert<int>(std::distance(m_MenuList.begin(), line)), false);
                    PushEvent(EVENT_DATACHANGED);
                }
                
                return true;
            }
    }

    return false;
}

int CMenu::CoreRequestWidth()
{
    if (m_iMaxWidth)
        return m_iMaxWidth;

    if (m_LongestLine)
        return SafeConvert<int>(m_LongestLine);
    
    return 3;
}

int CMenu::CoreRequestHeight()
{
    if (m_iMaxHeight)
        return m_iMaxHeight;

    if (!m_MenuList.empty())
        return SafeConvert<int>(m_MenuList.size());

    return 3;
}

void CMenu::CoreScroll(int vscroll, int hscroll)
{
    m_iXOffset = hscroll;
    m_iYOffset = vscroll;
}

CMenu::TScrollRange CMenu::CoreGetRange()
{
    return TScrollRange(SafeConvert<int>(m_MenuList.size()), SafeConvert<int>(m_LongestLine));
}

CMenu::TScrollRange CMenu::CoreGetScrollRegion()
{
    return TScrollRange(ScrollFieldHeight(), ScrollFieldWidth());
}

void CMenu::AddEntry(const std::string &id, const std::string &name)
{
    m_MenuList.insert(std::lower_bound(m_MenuList.begin(), m_MenuList.end(), name), SEntry(id, name));
    RequestQueuedDraw();
}

void CMenu::Select(const std::string &id)
{
    TMenuList::iterator line = std::lower_bound(m_MenuList.begin(), m_MenuList.end(), id);

    if ((line != m_MenuList.end()) && (line->id == id))
    {
        VScroll(SafeConvert<int>(std::distance(m_MenuList.begin(), line)), false);
        PushEvent(EVENT_DATACHANGED);
    }
}

void CMenu::Clear()
{
    m_MenuList.clear();
    m_iXOffset = m_iYOffset = m_iCursorLine = 0;
    RequestQueuedDraw();
    PushEvent(EVENT_DATACHANGED);
}

void CMenu::SetName(const std::string &id, const std::string &name)
{
    TMenuList::iterator line = std::lower_bound(m_MenuList.begin(), m_MenuList.end(), id);

    if ((line != m_MenuList.end()) && (line->id == id))
        line->name = name;
}

}

