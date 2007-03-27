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
#include "label.h"
#include "buttonbar.h"

namespace NNCurses {

// -------------------------------------
// Button Bar Class
// -------------------------------------

void CButtonBar::PushBox(void)
{
    m_pCurBox = new CBox(HORIZONTAL, false);
    AddWidget(m_pCurBox);
}

void CButtonBar::PushLabel(const std::string &n, const std::string &d)
{
    CBox *lbox = new CBox(HORIZONTAL, false);
    
    CLabel *label = new CLabel(n + ": ");
    label->SetDFColors(GetFColors());
    lbox->StartPack(label, false, false, 0);
    
    label = new CLabel(d);
    label->SetDFColors(GetDFColors());
    lbox->StartPack(label, false, false, 0);

    m_pCurBox->StartPack(lbox, true, false, 0);
}

void CButtonBar::CoreDraw()
{
    if (!m_QueuedEntries.empty())
    {
        if (!m_pCurBox)
            PushBox();
        
        while (!m_QueuedEntries.empty())
        {
            SButtonEntry entry = m_QueuedEntries.front();
            
            int fwidth = FieldWidth();
            std::string text = entry.name + ": " + entry.description;
            int tlength = SafeConvert<int>(text.length());
        
            if (fwidth < tlength)
                ; // UNDONE: Exception
            else if ((m_pCurBox->RequestWidth() + tlength) > fwidth)
                PushBox();
            
            assert(fwidth >= tlength);
            
            PushLabel(entry.name, entry.description);
            
            m_QueuedEntries.pop_front();
        }
        PushEvent(EVENT_REQUPDATE);
    }
    else
        CBox::CoreDraw();

//     CBox::DrawLayout();
}

void CButtonBar::AddButton(const std::string &n, const std::string &d)
{
//     m_QueuedEntries.push_back(SButtonEntry(n, d));
//     UpdateLayout();
    if (!m_pCurBox)
        PushBox();
    
    int fwidth = /*FieldWidth();*/GetWWidth(stdscr);
    std::string text = n + ": " + d;
    int tlength = SafeConvert<int>(text.length());

    if (fwidth < tlength)
        ; // UNDONE: Exception
    else if ((m_pCurBox->RequestWidth() + tlength) > fwidth)
        PushBox();
    
    assert(fwidth >= tlength);
    
    PushLabel(n, d);
    UpdateLayout();
    PushEvent(EVENT_REQUPDATE);
}

}
