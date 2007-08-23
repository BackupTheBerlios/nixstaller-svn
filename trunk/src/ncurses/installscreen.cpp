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

#include "main/main.h"
#include "installscreen.h"
#include "luagroup.h"
#include "tui/label.h"

// -------------------------------------
// NCurses Install Screen Class
// -------------------------------------

CInstallScreen::CInstallScreen(const std::string &title) : CBaseScreen(title), CBox(NNCurses::CBox::VERTICAL, false, 1),
                                                           m_pTitle(NULL), m_pGroupBox(NULL), m_WidgetRange(NULL, NULL)
{
    m_pTopBox = new NNCurses::CBox(HORIZONTAL, false);
    
    if (!title.empty())
    {
        m_pTopBox->AddWidget(m_pTitle = new NNCurses::CLabel(title));
        m_pTitle->SetMaxReqWidth(NNCurses::GetMaxWidth() - 7);
    }
    
    m_pTopBox->EndPack(m_pCounter = new NNCurses::CLabel(""), false, false, 0, 1);
    m_pCounter->SetDFColors(COLOR_RED, COLOR_BLUE);
    m_pCounter->Enable(false);
    
    if (!m_pTitle)
        m_pTopBox->Enable(false);
    
    StartPack(m_pTopBox, false, false, 0, 0);
}

CBaseLuaGroup *CInstallScreen::CreateGroup()
{
    if (!m_pGroupBox)
        AddWidget(m_pGroupBox = new NNCurses::CBox(VERTICAL, false));
    
    CLuaGroup *ret = new CLuaGroup();
    m_pGroupBox->StartPack(ret, true, true, 0, 1);
    return ret;
}

void CInstallScreen::CoreUpdateLanguage(void)
{
    if (m_pTitle)
        m_pTitle->SetText(GetTranslation(GetTitle()));
}

int CInstallScreen::CheckWidgetHeight(NNCurses::CWidget *w)
{
    int ret = m_pGroupBox->GetTotalWidgetH(w);
    
    if (ret > MaxScreenHeight())
        throw Exceptions::CExOverflow("Not enough space for widget.");
    
    return ret;
}

void CInstallScreen::ResetWidgetRange()
{
    m_WidgetRange.first = m_WidgetRange.second = NULL;
    
/*    if (!GetWin())
        return; // Not initialized yet*/
    
    if (m_pGroupBox)
    {
        int h = 0;
        const TChildList &list = m_pGroupBox->GetChildList();
        
        for (TChildList::const_iterator it=list.begin(); it!=list.end(); it++)
        {
            CGroup *g = m_pGroupBox->GetGroupWidget(*it);
            if (g && g->Empty())
                continue;
            
            if (h <= MaxScreenHeight())
            {
                h += CheckWidgetHeight(*it);
                if (h <= MaxScreenHeight())
                {
                    (*it)->Enable(true);
                    m_WidgetRange.second = *it;
                    continue;
                }
            }
            
            (*it)->Enable(false);
        }
    }

    UpdateCounter();
}

int CInstallScreen::MaxScreenHeight() const
{
    // 7: Additional height needed by window frames and such (hacky)
    // +1: Spacing from title label
    return NNCurses::GetMaxHeight() - (7 + std::max(1, m_pTopBox->RequestHeight()) + 1);
}

void CInstallScreen::UpdateCounter()
{
    if (!m_pGroupBox)
    {
        m_pCounter->Enable(false);
        return;
    }
    
    int count = 1, current = 1, h = 0;
    const TChildList &list = m_pGroupBox->GetChildList();
    
    for (TChildList::const_iterator it=list.begin(); it!=list.end(); it++)
    {
        CGroup *g = m_pGroupBox->GetGroupWidget(*it);
        if (g && g->Empty())
            continue;

        const int wh = CheckWidgetHeight(*it);
        
        if ((h + wh) > MaxScreenHeight())
        {
            count++;
            h = 0;
        }
        
        h += wh;
        
        if (*it == m_WidgetRange.first)
            current = count;
    }
    
    if (count > 1)
        m_pCounter->SetText(CreateText("%d/%d", current, count));
    
    m_pCounter->Enable(count > 1);
    m_pTopBox->Enable((count > 1) || m_pTitle);
}

bool CInstallScreen::CheckWidgets()
{
    if (!m_pGroupBox)
        return true;
    
    const TChildList &list = m_pGroupBox->GetChildList();
    TChildList::const_iterator start, end;
    
    if (m_WidgetRange.first)
        start = std::find(list.begin(), list.end(), m_WidgetRange.first);
    else
        start = list.begin();
    
    if (m_WidgetRange.second)
        end = std::find(list.begin(), list.end(), m_WidgetRange.second);
    else
        end = list.end()-1;

    while (true)
    {
        CLuaGroup *w = dynamic_cast<CLuaGroup *>(*start);
        if (w && !w->CheckWidgets())
            return false;
        
        if (start == end)
            break;
        
        start++;
    }
    
    return true;
}

bool CInstallScreen::SubBack()
{
    if (HasPrevWidgets())
    {
        const TChildList &list = m_pGroupBox->GetChildList();
        TChildList::const_reverse_iterator it = list.rbegin();
        
        for (; it!=list.rend(); it++)
            (*it)->Enable(false);
        
        int h = 0;
        it = std::find(list.rbegin(), list.rend(), m_WidgetRange.first);
        m_WidgetRange.first = m_WidgetRange.second = NULL;
        
        if (it != list.rend())
        {
            while ((*it != list.front()) && (h <= MaxScreenHeight()))
            {
                it++;
                
                CGroup *g = m_pGroupBox->GetGroupWidget(*it);
                if (g && g->Empty())
                    continue;

                h += CheckWidgetHeight(*it);
                
                if (h <= MaxScreenHeight())
                {
                    (*it)->Enable(true);
                    m_WidgetRange.first = *it;
                }
                
                if (!m_WidgetRange.second)
                    m_WidgetRange.second = *it;
            }
        }
        
        if (h)
        {
            UpdateCounter();
            return true;
        }
    }
    
    return false;
}

bool CInstallScreen::SubNext()
{
    if (!CheckWidgets())
        return true; // Return true so the current install screen doesn't change
    
    if (HasNextWidgets())
    {
        const TChildList &list = m_pGroupBox->GetChildList();
        TChildList::const_iterator it = list.begin();
        
        for (; it!=list.end(); it++)
            (*it)->Enable(false);
        
        int h = 0;
        it = std::find(list.begin(), list.end(), m_WidgetRange.second);
        m_WidgetRange.first = m_WidgetRange.second = NULL;
        
        if (it != list.end())
        {
            while ((*it != list.back()) && (h <= MaxScreenHeight()))
            {
                it++;
                
                CGroup *g = m_pGroupBox->GetGroupWidget(*it);
                if (g && g->Empty())
                    continue;

                h += CheckWidgetHeight(*it);
                
                if (h <= MaxScreenHeight())
                {
                    (*it)->Enable(true);
                    m_WidgetRange.second = *it;
                }
                
                if (!m_WidgetRange.first)
                    m_WidgetRange.first = *it;
            }
        }
        
        if (h)
        {
            UpdateCounter();
            SetNextFocWidget(false); // Focus first widget
            return true;
        }
    }
    
    return false;
}
