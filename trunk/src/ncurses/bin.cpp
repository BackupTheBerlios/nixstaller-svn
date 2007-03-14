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

#include "ncurses.h"
#include "widget.h"
#include "group.h"
#include "bin.h"

namespace NNCurses {

// -------------------------------------
// Bin Group Class
// -------------------------------------

void CBin::UpdateMySize(CWidget *w)
{
    int width = GetIdealW(w);
    int height = GetIdealH(w);
    
    if (HasBox())
    {
        width += 2;
        height += 2;
    }
    
    SetSize(X(), Y(), width, height);
}

void CBin::UpdateWidgetSize(CWidget *w)
{
    int x = 0, y = 0;
    int width = Width();
    int height = Height();
    
    if (HasBox())
    {
        x++;
        y++;
        width -= 2;
        height -= 2;
    }
    
    w->SetSize(x, y, width, height);
}

bool CBin::HandleEvent(CWidget *emitter, int type)
{
    if ((emitter == GetChild()) && (type == EVENT_REQSIZECHANGE))
    {
        m_bSync = true;
        PushEvent(EVENT_REQSIZECHANGE);
        return true;
    }
    
    return false;
}

void CBin::CoreAddWidget(CWidget *w)
{
    Clear(); // Bin can only contain one other widget
    m_bSync = true;
    InitChild(w);
}

int CBin::CoreRequestWidth()
{
    if (Empty())
        return GetMinWidth();
    
    return GetIdealW(GetChild());
}

int CBin::CoreRequestHeight()
{
    if (Empty())
        return GetMinHeight();
    
    return GetIdealH(GetChild());
}

void CBin::CoreDraw()
{
    if (m_bSync)
    {
        if (!Empty())
        {
            CWidget *w = GetChild();
            UpdateMySize(w);
            UpdateWidgetSize(w);
        }
        
        m_bSync = false;
    }
    
    CGroup::CoreDraw();
}

}
