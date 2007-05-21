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

CTextBase::CTextBase(bool c, bool w) : m_bCenter(c), m_bWrap(w), m_iMaxReqWidth(0),
                                       m_iMaxReqHeight(0), m_LongestLine(0), m_iCurWidth(0)
{
}

TSTLStrSize CTextBase::GetNextLine(const std::string &text, TSTLStrSize start, int width)
{
    TSTLStrSize length = text.length(), end = 0;
    
    if (m_bWrap)
    {
        end = start + width;
        
        if (end < start) // Overflow
            end = length - 1;
        
        if (end >= length)
            end = length - 1;
        
        TSTLStrSize newline = text.find("\n", start);
        if (newline < end)
            end = newline;
    
        if ((((end-start)+1) > SafeConvert<TSTLStrSize>(width)) && !isspace(m_QueuedText[end]))
        {
            std::string sub = m_QueuedText.substr(start, (end-start)+1);
            TSTLStrSize pos = sub.find_last_of(" \t\n");
            
            if (pos != std::string::npos)
                end = start + pos;
        }
    
    }
    else
    {
        end = m_QueuedText.find("\n", start);
            
        if (end == std::string::npos)
            end = length;
    }
    
    return end;
}

void CTextBase::UpdateText(int width)
{
    TSTLStrSize length = m_QueuedText.length(), start = 0, end;
    
    if (!m_Lines.empty() && (m_Lines.back().rfind("\n") == std::string::npos))
    {
        // This makes adding text easier
        m_QueuedText = m_Lines.back() + m_QueuedText;
        m_Lines.pop_back();
    }
    
    while (start < length)
    {
        end = GetNextLine(m_QueuedText, start, width);
        
        TSTLStrSize newlen = (end-start)+1;
        m_Lines.push_back(m_QueuedText.substr(start, newlen));
            
        if (newlen > m_LongestLine)
            m_LongestLine = newlen;
            
        start = end + 1;
    }
    
    m_QueuedText.clear();
}

void CTextBase::CoreDraw()
{
    if (!m_QueuedText.empty())
        UpdateText(Width());
    
    DrawWidget();
}

int CTextBase::CoreRequestWidth()
{
    int min = std::max(1, GetMinWidth());
    
    if (m_LongestLine)
        min = std::max(min, SafeConvert<int>(m_LongestLine));
    
    if (!m_QueuedText.empty())
    {
        TSTLStrSize longest = 0, start = 0, end, length = m_QueuedText.length();
        
        do
        {
            end = m_QueuedText.find("\n", start);
            
            if (end == std::string::npos)
                end = length-1;
            
            longest = std::max(longest, (end - start)+1);
            start = end + 1;
        }
        while ((start < length) && (start <= end));
        
        min = std::max(min, SafeConvert<int>(longest));
    }
    
    if ((m_iMaxReqWidth > 0) && (min > m_iMaxReqWidth))
        min = m_iMaxReqWidth;
    
    return min;
}

int CTextBase::CoreRequestHeight()
{
    int min = std::max(1, GetMinHeight());
    
    if (!m_Lines.empty())
        min = std::max(min, SafeConvert<int>(m_Lines.size()));
    
    if (!m_QueuedText.empty())
    {
        int lines = 0;
        const int width = RequestWidth();
        TSTLStrSize start = 0, end, length = m_QueuedText.length();
        
        while (start < length)
        {
            end = GetNextLine(m_QueuedText, start, width);
        
            lines++;
            start = end + 1;
        }
        
        min = std::max(min, lines);
    }
    
    if ((m_iMaxReqHeight > 0) && (min > m_iMaxReqHeight))
        min = m_iMaxReqHeight;
    
    return min;
}

void CTextBase::UpdateSize()
{
    if (Width() != m_iCurWidth)
    {
        // Big update....
        std::string oldqueue = m_QueuedText;
        m_QueuedText.clear();
        
        for (TLinesList::iterator it = m_Lines.begin(); it != m_Lines.end(); it++)
            m_QueuedText += *it;
        
        m_QueuedText += oldqueue;
        m_Lines.clear();
        m_iCurWidth = Width();
    }
}

void CTextBase::AddText(std::string t)
{
    ConvertTabs(t);
    m_QueuedText += t;
    RequestQueuedDraw();
}

void CTextBase::Clear()
{
    m_LongestLine = 0;
    m_Lines.clear();
    m_QueuedText.clear();
    RequestQueuedDraw();
}

}
