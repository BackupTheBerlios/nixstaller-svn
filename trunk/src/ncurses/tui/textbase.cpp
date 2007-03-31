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
#include "textbase.h"

namespace NNCurses {

// -------------------------------------
// Text Base Class
// -------------------------------------

CTextBase::CTextBase(bool c, bool w) : m_bCenter(c), m_bWrap(w), m_iMaxWidth(0),
                                       m_iMaxHeight(0), m_LongestLine(0)
{
}

void CTextBase::UpdateText()
{
    TSTLStrSize length = m_QueuedText.length(), start = 0;
    
    if (!m_Lines.empty() && (m_Lines.back().find("\n") == std::string::npos))
    {
        // This makes adding text easier
        m_QueuedText = m_Lines.back() + m_QueuedText;
        m_Lines.pop_back();
    }
    
    while (start < length)
    {
        TSTLStrSize end = start + Width();
        
        if (end < start) // Overflow
            end = length - 1;
        
        if (end >= length)
            end = length - 1;
        
        TSTLStrSize newline = m_QueuedText.find("\n", start);
        if (newline < end)
            end = newline;
        
        if ((((end-start)+1) > SafeConvert<TSTLStrSize>(Width())) && !isspace(m_QueuedText[end]))
        {
            std::string sub = m_QueuedText.substr(start, (end-start)+1);
            TSTLStrSize pos = sub.find_last_of(" \t\n");
            
            if (pos != std::string::npos)
                end = start + pos;
        }
        
        TSTLStrSize newlen = (end-start)+1;
        m_Lines.push_back(m_QueuedText.substr(start, newlen));
        
        if (newlen > m_LongestLine)
            m_LongestLine = newlen;
        
        start = m_QueuedText.find_first_not_of(" \t\n", end+1);
    }
    
    m_QueuedText.clear();
}

void CTextBase::CoreDraw()
{
    if (!m_QueuedText.empty())
        UpdateText();
    
    DrawWidget();
}

int CTextBase::CoreRequestWidth()
{
    if (m_iMaxWidth)
        return m_iMaxWidth;

    if (m_LongestLine)
        return SafeConvert<int>(m_LongestLine);
    
    return SafeConvert<int>(m_QueuedText.length());
}

int CTextBase::CoreRequestHeight()
{
    if (m_iMaxHeight)
        return m_iMaxHeight;

    if (!m_Lines.empty())
        return SafeConvert<int>(m_Lines.size());

    return 1;
}

}
