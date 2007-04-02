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

#ifndef TEXTWIDGET
#define TEXTWIDGET

#include "group.h"
#include "textbase.h"

namespace NNCurses {

class CTextWidget: public CTextBase
{
    int m_iXOffset, m_iYOffset;
    
protected:
    virtual void DoDraw(void);
    virtual bool CoreCanFocus(void) { return true; }
    virtual void DrawLine(int y, TLinesList::iterator it);
    
public:
    CTextWidget(bool w) : CTextBase(false, w), m_iXOffset(0), m_iYOffset(0) { }
    void SetOffset(int x, int y) { m_iXOffset = x; m_iYOffset = y; }
    std::pair<int, int> GetRange(void);
};


}

#endif
