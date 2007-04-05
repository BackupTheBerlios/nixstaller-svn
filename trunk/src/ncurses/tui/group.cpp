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
#include <algorithm>
#include "tui.h"
#include "widget.h"
#include "group.h"

namespace NNCurses {

// -------------------------------------
// Group Widget Class
// -------------------------------------

bool CGroup::CanFocusChilds(CWidget *w)
{
    return (IsGroupWidget(w) && !w->CanFocus() && !GetGroupWidget(w)->Empty());
}

void CGroup::CoreDraw(void)
{
    if (m_bUpdateLayout)
    {
        DrawLayout();
        m_bUpdateLayout = false;
    }
    
    DrawWidget();
    DrawChilds();
}

void CGroup::UpdateSize()
{
    for (TChildList::iterator it=m_Childs.begin(); it!=m_Childs.end(); it++)
    {
        if (!(*it)->GetWin()) // Widget isn't created yet (size will be set when widget is created)
            continue;
        
        int ret = mvwin((*it)->GetWin(), Y()+(*it)->Y(), X()+(*it)->X());
        assert(ret != ERR);
        // UNDONE: Exception
    }
}

void CGroup::UpdateFocus()
{
    if (m_pFocusedWidget)
        m_pFocusedWidget->Focus(Focused());
    
    for (TChildList::iterator it=m_Childs.begin(); it!=m_Childs.end(); it++)
    {
        if (*it == m_pFocusedWidget)
            continue;
        
        // Let all other widgets update their colors. The group has changed it's color which may affect any childs
        (*it)->TouchColor();
        (*it)->RequestQueuedDraw();
    }
}

bool CGroup::CoreHandleKey(chtype key)
{
    if (m_pFocusedWidget)
        return m_pFocusedWidget->HandleKey(key);

    return false;
}

void CGroup::CoreDrawChilds(void)
{
    for (TChildList::iterator it=m_Childs.begin(); it!=m_Childs.end(); it++)
    {
        if ((*it)->Enabled())
            (*it)->Draw();
    }
}

void CGroup::CoreGetButtonDescs(TButtonDescList &list)
{
    if (m_pFocusedWidget)
        m_pFocusedWidget->GetButtonDescs(list);
}

void CGroup::AddWidget(CGroup *g)
{
    m_GroupMap[g] = g;
    AddWidget(static_cast<CWidget *>(g));
}

void CGroup::InitChild(CWidget *w)
{
    m_Childs.push_back(w);
    w->SetParent(this);
    
    if (!m_pFocusedWidget && Focused() && (w->CanFocus() || CanFocusChilds(w)))
        FocusWidget(w);
}

void CGroup::RemoveWidget(CWidget *w)
{
    CoreRemoveWidget(w);
    
    if (m_pFocusedWidget == w)
        SetValidWidget(w);
    
    TChildList::iterator it = std::find(m_Childs.begin(), m_Childs.end(), w);
    assert(it != m_Childs.end());
    m_Childs.erase(it);
    m_GroupMap[w] = NULL; // Is NULL already if w isn't a group
}

void CGroup::Clear()
{
    while (!m_Childs.empty())
    {
        // CWidget's destructor will call RemoveWidget() which removes widget from m_Childs list
        delete m_Childs.back();
    }

    m_GroupMap.clear();
}

void CGroup::FocusWidget(CWidget *w)
{
    if (m_pFocusedWidget)
    {
        m_pFocusedWidget->Focus(false);
        m_pFocusedWidget->RequestQueuedDraw();
    }
    
    m_pFocusedWidget = w;
    
    if (w) // If w == NULL the current focused widget is reset
    {
        w->Focus(true);
        w->RequestQueuedDraw();
    }
}

bool CGroup::SetNextFocWidget(bool cont)
{
    if (m_Childs.empty())
        return false;

    TChildList::iterator it = m_Childs.begin();
    
    if (cont && m_pFocusedWidget && Focused())
    {
        TChildList::iterator f = std::find(m_Childs.begin(), m_Childs.end(), m_pFocusedWidget);
        if (f != m_Childs.end())
        {
            it = f;
            if (!CanFocusChilds(*it))
                it++;
        }
    }
    
    for (; it!=m_Childs.end(); it++)
    {
        if (!(*it)->CanFocus())
        {
            if (CanFocusChilds(*it))
            {
                if (!GetGroupWidget(*it)->SetNextFocWidget(cont))
                    continue;
            }
            else
                continue;
        }

        FocusWidget(*it);
        return true;
    }
    
    return false;
}

bool CGroup::SetPrevFocWidget(bool cont)
{
    if (m_Childs.empty())
        return false;

    TChildList::reverse_iterator it = m_Childs.rbegin();
    
    if (cont && m_pFocusedWidget && Focused())
    {
        TChildList::reverse_iterator f = std::find(m_Childs.rbegin(), m_Childs.rend(), m_pFocusedWidget);
        if (f != m_Childs.rend())
        {
            it = f;
            if (!CanFocusChilds(*it))
                it++;
        }
    }
    
    for (; it!=m_Childs.rend(); it++)
    {
        if (!(*it)->CanFocus())
        {
            if (CanFocusChilds(*it))
            {
                if (!GetGroupWidget(*it)->SetPrevFocWidget(cont))
                    continue;
            }
            else
                continue;
        }

        FocusWidget(*it);
        return true;
    }
    
    return false;
}

void CGroup::SetValidWidget(CWidget *ignore)
{
    TChildList::iterator itprev, itnext;
    
    itprev = itnext = std::find(m_Childs.begin(), m_Childs.end(), ignore);
    
    do
    {
        if (itprev != m_Childs.begin())
        {
            itprev--;
            
            if ((*itprev)->CanFocus() || CanFocusChilds(*itprev))
            {
                FocusWidget(*itprev);
                return;
            }
        }
        
        if (*itnext != m_Childs.back())
        {
            itnext++;
            
            if (itnext != m_Childs.end())
            {
                if ((*itnext)->CanFocus() || CanFocusChilds(*itnext))
                {
                    FocusWidget(*itnext);
                    return;
                }
            }
        }
    }
    while ((itprev != m_Childs.begin()) || (*itnext != m_Childs.back()));
    
    // Haven't found a focusable widget
    m_pFocusedWidget = NULL;
}

}
