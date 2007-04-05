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

#include <assert.h>
#include "tui.h"
#include "box.h"

namespace NNCurses {

// -------------------------------------
// Widget Box Class
// -------------------------------------

int CBox::GetWidgetW(CWidget *w)
{
    int ret = 0;
    
    if (m_bEqual)
    {
        // UNDONE: Cache?
        TChildList childs = GetChildList();
        
        for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
            ret = std::max(ret, (*it)->RequestWidth());
    }
    else
        ret = w->RequestWidth();
    
    return ret;
}

int CBox::GetWidgetH(CWidget *w)
{
    int ret = 0;
    
    if (m_bEqual)
    {
        // UNDONE: Cache?
        TChildList childs = GetChildList();
        
        for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
            ret = std::max(ret, (*it)->RequestHeight());
    }
    else
        ret = w->RequestHeight();
    
    return ret;
}

int CBox::RequestedWidgetsW()
{
    TChildList childs = GetChildList();
    int ret = 0;
    
    for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
    {
        if (!IsValidWidget(*it))
            continue;
        
        if (m_eDirection == HORIZONTAL)
        {
            const SBoxEntry &entry = m_BoxEntries[*it];

            ret += GetWidgetW(*it) + (2 * entry.padding);
            if (*it != childs.back())
                ret += m_iSpacing;
        }
        else
            ret = std::max(ret, (*it)->RequestWidth());
    }
    
    return ret;
}

int CBox::RequestedWidgetsH()
{
    TChildList childs = GetChildList();
    int ret = 0;
    
    for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
    {
        if (!IsValidWidget(*it))
            continue;

        if (m_eDirection == VERTICAL)
        {
            const SBoxEntry &entry = m_BoxEntries[*it];

            ret += (GetWidgetH(*it) + (2 * entry.padding));
            
            if (*it != childs.back())
                ret += m_iSpacing;
        }
        else
            ret = std::max(ret, (*it)->RequestHeight());
    }
    
    return ret;
}

CBox::TChildList::size_type CBox::ExpandedWidgets()
{
    TChildList childs = GetChildList();
    TChildList::size_type ret = 0;
    
    for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
    {
        if (!IsValidWidget(*it))
            continue;
        
        if (!m_bEqual && !m_BoxEntries[*it].expand)
            continue;
        
        ret++;
    }
    
    return ret;
}

bool CBox::IsValidWidget(CWidget *w)
{
    return w->Enabled();
}

void CBox::CoreDrawLayout()
{
    if (Empty())
        return;

    TChildList childs = GetChildList();
    const TChildList::size_type size = ExpandedWidgets();
    const int diffw = FieldWidth() - RequestedWidgetsW();
    const int diffh = FieldHeight() - RequestedWidgetsH();
    const int extraw = (size) ? diffw / size : 0;
    const int extrah = (size) ? diffh / size : 0;
    int remainingw = (size) ? diffw % size : 0;
    int remainingh = (size) ? diffh % size : 0;
    int begx = FieldX(), begy = FieldY(); // Coords for widgets that were packed at start
    int endx = (FieldX()+FieldWidth())-1, endy = (FieldY()+FieldHeight())-1; // Coords for widgets that were packed at end

    for (TChildList::iterator it=childs.begin(); it!=childs.end(); it++)
    {
        if (!IsValidWidget(*it))
            continue;
        
        int basex = 0, basey = 0;
        int widgetw = 0, widgeth = 0;
        int basew = extraw, baseh = extrah;
        const SBoxEntry &entry = m_BoxEntries[*it];
        int spacing = entry.padding;
    
        if (*it != childs.back())
            spacing += m_iSpacing;
    
        if (remainingw)
        {
            basew++;
            remainingw--;
        }
    
        if  (remainingh)
        {
            baseh++;
            remainingh--;
        }
    
        if (m_eDirection == HORIZONTAL)
        {
            basex += entry.padding;
            widgetw += GetWidgetW(*it);
            widgeth += FieldHeight();
        
            if (m_bEqual || entry.expand)
            {
                if (entry.fill)
                    widgetw += basew;
                else
                {
                    spacing += (basew/2);
                    basex += (basew/2);
                }
            }
        }
        else
        {
            basey += entry.padding;
            widgetw += FieldWidth();
            widgeth += GetWidgetH(*it);
        
            if (entry.expand)
            {
                if (entry.fill)
                    widgeth += baseh;
                else
                {
                    spacing += (baseh/2);
                    basey += (baseh/2);
                }
            }
        }
    
        assert(widgetw>0 && widgeth>0);
        assert(widgetw>=(*it)->RequestWidth() && widgeth>=(*it)->RequestHeight());
        
        if (entry.start)
        {
            begx += basex;
            begy += basey;
        
            SetChildSize(*it, begx, begy, widgetw, widgeth);
        
            if (m_eDirection == HORIZONTAL)
                begx += (spacing + widgetw);
            else
                begy += (spacing + widgeth);
        }
        else
        {
            endx -= basex;
            endy -= basey;
        
            SetChildSize(*it, endx-(widgetw-1), endy-(widgeth-1), widgetw, widgeth);
        
            if (m_eDirection == HORIZONTAL)
                endx -= (spacing + widgetw);
            else
                endy -= (spacing + widgeth);
        }
    }
}

int CBox::CoreRequestWidth()
{
    int ret = RequestedWidgetsW();
    
    if (HasBox())
        ret += 2;
    
    return std::max(ret, GetMinWidth());
}

int CBox::CoreRequestHeight()
{
    int ret = RequestedWidgetsH();
    
    if (HasBox())
        ret += 2;

    return std::max(ret, GetMinHeight());
}

bool CBox::CoreHandleEvent(CWidget *emitter, int event)
{
    if (IsDirectChild(emitter, this))
    {
        if (event == EVENT_REQUPDATE)
        {
            UpdateLayout();
            
            PushEvent(event);
            
            if (!GetParentWidget())
                RequestQueuedDraw();
            
            return true;
        }
    }
    
    return false;
}

void CBox::CoreRemoveWidget(CWidget *w)
{
    UpdateLayout();
    PushEvent(EVENT_REQUPDATE);
}

void CBox::StartPack(CGroup *g, bool e, bool f, int p)
{
    m_BoxEntries[g] = SBoxEntry(e, f, p, true);
    CGroup::AddWidget(g);
}

void CBox::StartPack(CWidget *w, bool e, bool f, int p)
{
    m_BoxEntries[w] = SBoxEntry(e, f, p, true);
    CGroup::AddWidget(w);
}

void CBox::EndPack(CGroup *g, bool e, bool f, int p)
{
    m_BoxEntries[g] = SBoxEntry(e, f, p, false);
    CGroup::AddWidget(g);
}

void CBox::EndPack(CWidget *w, bool e, bool f, int p)
{
    m_BoxEntries[w] = SBoxEntry(e, f, p, false);
    CGroup::AddWidget(w);
}

}
