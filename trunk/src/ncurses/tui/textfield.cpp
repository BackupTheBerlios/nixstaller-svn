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
#include "textfield.h"
#include "textwidget.h"
#include "scrollbar.h"

namespace NNCurses {

// -------------------------------------
// Text Field Class
// -------------------------------------

CTextField::CTextField(int maxw, int maxh, bool w)
{
    m_pTextWidget = new CTextWidget(w);
    
    if (maxw)
        m_pTextWidget->SetMaxReqWidth(maxw);
    
    if (maxh)
        m_pTextWidget->SetMaxReqHeight(maxh);
    
    AddWidget(m_pTextWidget);
}

bool CTextField::CoreHandleKey(chtype key)
{
    if (CBaseScroll::CoreHandleKey(key))
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
            HScroll(m_pTextWidget->GetWRange()-m_pTextWidget->Width(), false);
            return true;
    }
    
    return false;
}

int CTextField::CoreRequestWidth()
{
    return m_pTextWidget->RequestWidth() + 2; // +2 For surrounding box
}

int CTextField::CoreRequestHeight()
{
    return m_pTextWidget->RequestHeight() + 2; // +2 For surrounding box
}

void CTextField::CoreDrawLayout()
{
    SetChildSize(m_pTextWidget, 1, 1, Width()-2, Height()-2);
    CBaseScroll::CoreDrawLayout();
}

void CTextField::CoreScroll(int vscroll, int hscroll)
{
    m_pTextWidget->SetOffset(hscroll, vscroll);
}

CTextField::TScrollRange CTextField::CoreGetRange()
{
    return TScrollRange(SafeConvert<int>(m_pTextWidget->GetHRange()), SafeConvert<int>(m_pTextWidget->GetWRange()));
}

CTextField::TScrollRange CTextField::CoreGetScrollRegion()
{
    return TScrollRange(m_pTextWidget->Height(), m_pTextWidget->Width());
}

void CTextField::AddText(const std::string &t)
{
    m_pTextWidget->AddText(t);
}

void CTextField::ClearText()
{
    m_pTextWidget->Clear();
    RequestQueuedDraw();
}

void CTextField::LoadFile(const char *f)
{
    std::ifstream file(f);
    std::string buf;
    char c;
    
    while (file && file.get(c))
        buf += c;
    
    AddText(buf);
}

}
