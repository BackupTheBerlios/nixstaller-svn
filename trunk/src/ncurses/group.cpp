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
#include "tui.h"

namespace NNCurses {

// -------------------------------------
// Group Widget Class
// -------------------------------------

void CGroup::CoreDraw(void)
{
    DrawWidget();
    
    for (TChildList::iterator it=m_Childs.begin(); it!=m_Childs.end(); it++)
        (*it)->Draw();
}

void CGroup::UpdateFocus()
{
    if (m_pFocusedWidget)
        m_pFocusedWidget->Focus(Focused());
}

bool CGroup::CoreCanFocus()
{
    for (TChildList::iterator it=m_Childs.begin(); it!=m_Childs.end(); it++)
    {
        if ((*it)->CanFocus())
            return true;
    }
    
    return false;
}

bool CGroup::CoreHandleKey(chtype key)
{
    if (m_pFocusedWidget)
        return m_pFocusedWidget->HandleKey(key);

    return false;
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
    w->Init();
}

void CGroup::RemoveWidget(CWidget *w)
{
    if (m_pFocusedWidget == w)
        SetValidWidget(w);
    
    TChildList::iterator it = std::find(m_Childs.begin(), m_Childs.end(), w);
    assert(it != m_Childs.end());
    m_Childs.erase(it);
    
    delete w;
}

void CGroup::Clear()
{
    while (!m_Childs.empty())
    {
        delete m_Childs.back();
        m_Childs.pop_back();
    }
    m_GroupMap.clear();
}

void CGroup::FocusWidget(CWidget *w)
{
    if (m_pFocusedWidget)
    {
        m_pFocusedWidget->Focus(false);
        m_pFocusedWidget->Draw();
    }
    
    m_pFocusedWidget = w;
    
    if (w) // This allows resetting the current focused widget
    {
        w->Focus(true);
        w->Draw();
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
            if (!IsGroupWidget(*it))
                it++;
        }
    }
    
    for (; it!=m_Childs.end(); it++)
    {
//     if (!w->CanFocus())
//         continue; UNDONE: ENABLE
    
        if (IsGroupWidget(*it))
        {
            if (!GetGroupWidget(*it)->SetNextFocWidget(cont))
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
            if (!IsGroupWidget(*it))
                it++;
        }
    }
    
    for (; it!=m_Childs.rend(); it++)
    {
//     if (!w->CanFocus())
//         continue; UNDONE: ENABLE
    
        if (IsGroupWidget(*it))
        {
            if (!GetGroupWidget(*it)->SetPrevFocWidget(cont))
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
            
            if ((*itprev)->CanFocus())
            {
                FocusWidget(*itprev);
                return;
            }
        }
        
        itnext++;
        
        if (itnext != m_Childs.end())
        {
            if ((*itnext)->CanFocus())
            {
                FocusWidget(*itnext);
                return;
            }
        }
    }
    while ((itprev != m_Childs.begin()) || (*itnext != m_Childs.back()));
    
    // Haven't found a focusable widget
    m_pFocusedWidget = NULL;
}

}
