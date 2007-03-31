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
#include "windowmanager.h"

namespace NNCurses {

// -------------------------------------
// Window Manager Class
// -------------------------------------

bool CWindowManager::CoreHandleEvent(CWidget *emitter, int event)
{
    if (event == EVENT_REQQUEUEDDRAW)
    {
        CWidget *focwidget = GetFocusedWidget();
        if (focwidget)
        {
            if ((focwidget != emitter) && !IsChild(emitter, focwidget))
                focwidget->RequestQueuedDraw();
        }
        
        return true;
    }
    
    return false;
}

int CWindowManager::CoreRequestWidth()
{
    TChildList childs = GetChildList();
    int ret = GetMinWidth();
    
    for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
    {
        if ((*it)->Enabled())
            ret = std::max(ret, (*it)->RequestWidth());
    }
    
    return ret;
}

int CWindowManager::CoreRequestHeight()
{
    TChildList childs = GetChildList();
    int ret = GetMinHeight();
    
    for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
    {
        if ((*it)->Enabled())
            ret = std::max(ret, (*it)->RequestHeight());
    }
    
    return ret;
}

void CWindowManager::CoreDraw()
{
    while (!m_WidgetQueue.empty())
    {
        CWidget *w = m_WidgetQueue.front();
        const int width = w->RequestWidth(), height = w->RequestHeight();
        const int x = (Width() - width) / 2;
        const int y = (Height() - height) / 2;
    
        SetChildSize(w, x, y, width, height);
        
        m_WidgetQueue.pop_front();
    }

    CGroup::CoreDraw();
}

void CWindowManager::CoreDrawWidgets()
{
    TChildList childs = GetChildList();
    
    for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
    {
        if (*it == GetFocusedWidget())
            continue; // We draw this one as last, so it looks like it's on top
        DrawChild(*it);
    }
    
    CWidget *w = GetFocusedWidget();
    
    if (w)
        DrawChild(w);
}

void CWindowManager::CoreAddWidget(CWidget *w)
{
    m_WidgetQueue.push_back(w);
    InitChild(w);
    PushEvent(EVENT_REQUPDATE);
}

}

