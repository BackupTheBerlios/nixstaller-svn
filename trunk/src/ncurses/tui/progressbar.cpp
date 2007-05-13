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
#include "progressbar.h"

namespace NNCurses {

// -------------------------------------
// Progress Bar Class
// -------------------------------------

CProgressBar::CProgressBar(float min, float max) : m_fMin(min), m_fMax(max), m_fCurrent(min)
{
    SetMinWidth(1);
    SetMinHeight(3);
    SetBox(true);
}

void CProgressBar::EnableColors(bool e)
{
    TColorPair colors;
    
    colors.first = COLOR_BLACK;
    
    if (!has_colors())
        colors.second = COLOR_WHITE;
    else
        colors.second = COLOR_GREEN;
    
    SetColorAttr(colors, e);
}

void CProgressBar::DoDraw()
{
    float maxw = SafeConvert<float>(Width()) - 2.0f; // -2 for the borders
    const float range = m_fMax - m_fMin;
    float fac = (range > 0.0f) ? (m_fCurrent / range) : 0.0f;
    
    EnableColors(true);
    
    int fill = SafeConvert<int>(maxw * fac);
    
    for (int i=0; i<fill; i++)
        AddCh(this, i+1, 1, ' ');
    
    EnableColors(false);
}

}
