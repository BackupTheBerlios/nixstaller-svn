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

#include <fstream>
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
    m_pTextWidget = CreateTextWidget(w);
    
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
    SetChildSize(m_pVScrollbar, Width()-1, 1, 1, Height()-2);
    SetChildSize(m_pHScrollbar, 1, Height()-1, Width()-2, 1);
}

void CTextWindow::SyncBars()
{
    std::pair<int, int> range = m_pTextWidget->GetRange();
    
    range.first -= m_pTextWidget->Width();
    range.second -= m_pTextWidget->Height();
    
    if (range.first < 0)
        range.first = 0;
    
    if (range.second < 0)
        range.second = 0;
    
    if (range != m_CurRange)
    {
        m_pVScrollbar->SetRange(0, range.second);
        m_pHScrollbar->SetRange(0, range.first);
        m_CurRange = range;
    }
}

void CTextWindow::VScroll(int n, bool relative)
{
    if (relative)
        m_pVScrollbar->Scroll(n);
    else
        m_pVScrollbar->SetCurrent(n);
    
    m_pTextWidget->SetOffset(m_pHScrollbar->Value(), m_pVScrollbar->Value());
    RequestQueuedDraw();
}

void CTextWindow::HScroll(int n, bool relative)
{
    if (relative)
        m_pHScrollbar->Scroll(n);
    else
        m_pHScrollbar->SetCurrent(n);

    m_pTextWidget->SetOffset(m_pHScrollbar->Value(), m_pVScrollbar->Value());
    RequestQueuedDraw();
}

CTextWidget *CTextWindow::CreateTextWidget(bool w)
{
    return new CTextWidget(w);
}

void CTextWindow::CoreDraw()
{
    if (m_bUpdateLayout)
    {
        DrawLayout();
        m_bUpdateLayout = false;
    }
    
    CGroup::CoreDraw();
    SyncBars();
}

bool CTextWindow::CoreHandleKey(chtype key)
{
    if (CGroup::CoreHandleKey(key))
        return true;
    
    switch (key)
    {
        case KEY_UP:
            VScroll(-1, true);
            return true;
        case KEY_DOWN:
            VScroll(1, true);
            return true;
        case KEY_LEFT:
            HScroll(-1, true);
            return true;
        case KEY_RIGHT:
            HScroll(1, true);
            return true;
        case KEY_PPAGE:
            VScroll(-m_pTextWidget->Height(), true);
            return true;
        case KEY_NPAGE:
            VScroll(m_pTextWidget->Height(), true);
            return true;
        case KEY_HOME:
            HScroll(0, false);
            return true;
        case KEY_END:
        {
            std::pair<int, int> range = m_pTextWidget->GetRange();
            HScroll(range.first-m_pTextWidget->Width(), false);
            return true;
        }
    }
    
    return false;
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

void CTextWindow::LoadFile(const char *f)
{
    std::ifstream file(f);
    std::string buf;
    char c;
    
    while (file && file.get(c))
        buf += c;
        
    AddText(buf);
}


}
