/*
    Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)

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
#include "label.h"

namespace NNCurses {

// -------------------------------------
// Label Widget Class
// -------------------------------------

void CLabel::UpdateLines()
{
    m_Lines.clear();
    
    TSTLStrSize length = m_szText.length(), start = 0;
    int width = CoreRequestWidth();
    
    while (start < length)
    {
        TSTLStrSize end = start + width;
        
        if (end < start) // Overflow
            end = length - 1;
        
        if (end >= length)
            end = length - 1;
        
        if ((((end-start)+1) > width) && !isspace(m_szText[end]))
        {
            std::string sub = m_szText.substr(start, (end-start)+1);
            TSTLStrSize pos = sub.find_last_of(" \t\n");
            
            if (pos != std::string::npos)
                end = start + pos;
        }
        
        m_Lines.push_back(m_szText.substr(start, (end-start)+1));
        start = m_szText.find_first_not_of(" \t\n", end+1);
    }
}

int CLabel::CoreRequestWidth()
{
    if (GetMinWidth())
        return GetMinWidth();
    
    return m_szText.length();
}

int CLabel::CoreRequestHeight()
{
    if (GetMinHeight())
        return GetMinHeight();
    
    if (!m_Lines.empty())
        return SafeConvert<int>(m_Lines.size());
    
    return 1;
}

void CLabel::DoDraw()
{
    int x = 0, y = (Height() - SafeConvert<int>(m_Lines.size())) / 2;
    
    if (y < 0)
        y = 0;
    
    for (TLinesList::iterator it=m_Lines.begin(); it!=m_Lines.end(); it++, y++)
    {
        if (y == Height())
            break;

        if (m_bCenter)
            x = (Width() - it->length()) / 2;
        
        mvwaddstr(GetWin(), y, x, it->c_str());
    }
}

void CLabel::SetText(const std::string &t)
{
    m_szText = t;
    UpdateLines();
    
    if (GetMinWidth() == 0)
        PushEvent(EVENT_REQUPDATE);
}

}
