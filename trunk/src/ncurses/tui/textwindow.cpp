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
#include "textwindow.h"
#include "textwidget.h"
#include "scrollbar.h"

namespace NNCurses {

// -------------------------------------
// Text Window Class
// -------------------------------------

CTextWindow::CTextWindow(int maxw, int maxh, bool w) : m_bUpdateLayout(true), m_iCurrent(0)
{
    SetBox(true);
    m_pTextWidget = new CTextWidget(w);
    
    if (maxw)
        m_pTextWidget->SetMaxWidth(maxw);
    
    if (maxh)
        m_pTextWidget->SetMaxHeight(maxh);
    
    AddWidget(m_pTextWidget);
    
    AddWidget(m_pVScrollbar = new CScrollbar(CScrollbar::VERTICAL));
    AddWidget(m_pHScrollbar = new CScrollbar(CScrollbar::HORIZONTAL));
}

void CTextWindow::DrawLayout()
{
    SetChildSize(m_pTextWidget, 1, 1, Width()-2, Height()-2);
    
    std::pair<int, int> range = m_pTextWidget->GetRange();
    
    m_pVScrollbar->SetRange(0, range.second);
    SetChildSize(m_pVScrollbar, Width()-1, 1, 1, Height()-2);
    
    m_pHScrollbar->SetRange(0, range.first);
    SetChildSize(m_pHScrollbar, 1, Height()-1, Width()-2, 1);
}

void CTextWindow::CoreDraw()
{
    if (m_bUpdateLayout)
    {
        DrawLayout();
        m_bUpdateLayout = false;
    }
    
    CGroup::CoreDraw();
}

int CTextWindow::CoreRequestWidth()
{
    return m_pTextWidget->RequestWidth() + 2; // +2 for Box
}

int CTextWindow::CoreRequestHeight()
{
    return m_pTextWidget->RequestHeight() + 2; // +2 for Box
}

void CTextWindow::AddText(const std::string &t)
{
    m_pTextWidget->AddText(t);
}

}
