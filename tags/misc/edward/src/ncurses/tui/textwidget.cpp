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

#include "tui.h"
#include "textwidget.h"

namespace NNCurses {

// -------------------------------------
// Textwidget Class
// -------------------------------------

void CTextWidget::CoreDraw()
{
    if (!GetQueuedText().empty())
        UpdateText(Width());
    m_LineCount = LineCount();

    DrawWidget();
}

void CTextWidget::DoDraw()
{
    TLinesList &lines = GetLineList();
    const int size = SafeConvert<int>(lines.size());
    
    if (m_iYOffset > size) // May happen when lines are deleted for example
        m_iYOffset = size;
        
    int y = 0;
    TLinesList::iterator it=lines.begin() + m_iYOffset;
    
    for (; ((it!=lines.end()) && (y<Height())); it++, y++)
    {
        if (y >= Height())
            break;
        
        int len = SafeConvert<int>(it->length());
        if (len >= m_iXOffset)
        {
            int end = std::min(Width(), len-m_iXOffset);
            AddStr(this, 0, y, it->substr(m_iXOffset, end).c_str());
        }
    }
}


}
