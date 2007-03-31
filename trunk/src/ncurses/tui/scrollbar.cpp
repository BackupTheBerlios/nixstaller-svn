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

#include <math.h>
#include "main.h"
#include "tui.h"
#include "group.h"
#include "scrollbar.h"

namespace NNCurses {

// -------------------------------------
// Scrollbar Class
// -------------------------------------

void CScrollbar::SyncColors()
{
    TColorPair colors;
    CWidget *p = GetParentWidget();

    if (p->Focused())
        colors = GetParentWidget()->GetFColors();
    else
        colors = GetParentWidget()->GetDFColors();
    
    SetDFColors(colors.second, colors.first); // Reverse colors
}
    
void CScrollbar::CoreDraw()
{
    SyncColors();
    InitDraw();
    DoDraw();
    RefreshWidget();
}

void CScrollbar::DoDraw()
{
    float fac = 0.0f;
    float posx, posy;
    
    if ((m_fMax - m_fMin) > 0.0f)
        fac = m_fCurrent / (m_fMax-m_fMin);
    
    if (m_eType == VERTICAL)
    {
        posx = 0.0f;
        posy = (float)(Height()-1) * fac;
    }
    else
    {
        posx = (float)(Width()-1) * fac;
        posy = 0.0f;
    }
    
    if (posx < 0.0f)
        posx = 0.0f;
    if (posy < 0.0f)
        posy = 0.0f;
    
    mvwaddch(GetWin(), SafeConvert<int>(posy), SafeConvert<int>(posx), ACS_CKBOARD);
}


}
