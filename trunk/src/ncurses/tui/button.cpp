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
#include "widget.h"
#include "button.h"
#include "label.h"

namespace NNCurses {

// -------------------------------------
// Button Class
// -------------------------------------

void CButton::DoDraw()
{
    AddCh(this, 0, 0, '<');
    AddCh(this, Width()-1, 0, '>');
}

void CButton::UpdateColors()
{
    if (m_pLabel)
    {
        TColorPair colors;
        
        if (Focused())
            colors = GetFColors();
        else
            colors = GetDFColors();
        
        m_pLabel->SetDFColors(colors);
        m_pLabel->RequestQueuedDraw();
    }
}

bool CButton::CoreHandleKey(chtype ch)
{
    if (CBox::CoreHandleKey(ch))
        return true;
    
    if (IsEnter(ch))
    {
        PushEvent(EVENT_CALLBACK);
        return true;
    }
    
    return false;
}

void CButton::CoreGetButtonDescs(TButtonDescList &list)
{
    list.push_back(TButtonDescPair("ENTER", "Activate"));
}

void CButton::SetText(const std::string &title)
{
    if (!m_pLabel)
    {
        m_pLabel = new CLabel(title);
        AddWidget(m_pLabel);
    }
    else
        m_pLabel->SetText(title);
    
    SetMinWidth(SafeConvert<int>(title.length()) + m_iExtraWidth);
    RequestQueuedDraw();
}


}
