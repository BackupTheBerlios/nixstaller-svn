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
#include "dialog.h"
#include "button.h"
#include "separator.h"

namespace NNCurses {

// -------------------------------------
// Dialog Class
// -------------------------------------

CDialog::CDialog() : m_pButtonBox(NULL), m_bDone(false)
{
}

bool CDialog::CoreHandleEvent(CWidget *emitter, int event)
{
    if (CBox::CoreHandleEvent(emitter, event))
        return true;
    
    if (event == EVENT_CALLBACK)
    {
        if (IsChild(emitter, this))
        {
            m_bDone = true;
            return true;
        }
    }
    
    return false;
}

void CDialog::AddButton(CButton *button)
{
    if (!m_pButtonBox)
    {
        m_pButtonBox = new CBox(HORIZONTAL, true, 0);
        EndPack(m_pButtonBox, true, true, 0);
        EndPack(new CSeparator(' '), true, true, 0);
    }
    
    m_pButtonBox->StartPack(button, true, false, 0);
}

bool CDialog::Run()
{
    return (TUI.Run() && !m_bDone);
}


}
