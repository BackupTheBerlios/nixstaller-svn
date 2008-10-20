/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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

#include <algorithm>
#include "main/main.h"
#include "installscreen.h"
#include "luagroup.h"
#include "tui/label.h"

// -------------------------------------
// NCurses Install Screen Class
// -------------------------------------

CInstallScreen::CInstallScreen(const std::string &title) : CBaseScreen(title), CBox(NNCurses::CBox::VERTICAL, false, 1),
                                                           m_pTitle(NULL), m_pGroupBox(NULL), m_CurSubScreen(0),
                                                           m_bDirty(false), m_bCleaning(false)
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

bool CInstallScreen::VisibleWidget(NNCurses::CWidget *w)
{
    CGroup *g = m_pGroupBox->GetGroupWidget(w);
    if (!g)
        return true;
    
    if (g->Empty())
        return false;
        
    CLuaGroup *lg = dynamic_cast<CLuaGroup *>(g);
    return lg->IsVisible();
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
    m_WidgetRanges.clear();
    m_CurSubScreen = 0;

    
/*    if (!GetWin())
        return; // Not initialized yet*/
    
    if (m_pGroupBox)
    {
        int h = 0;
        bool enablewidgets = true;
        const TChildList &list = m_pGroupBox->GetChildList();
        
        if (list.empty())
            return;
        
        m_WidgetRanges.push_back(list.front());
        bool endedscreen = false;
        
        for (TChildList::const_iterator it=list.begin(); it!=list.end(); it++)
        {
            if (!VisibleWidget(*it))
            {
                (*it)->Enable(false);
                continue;
            }
            
            int newh = CheckWidgetHeight(*it);
            if (!endedscreen && ((newh + h) <= MaxScreenHeight()))
            {
                h += newh;
                (*it)->Enable(enablewidgets);
            }
            else
            {
                m_WidgetRanges.push_back(*it);
                h = newh;
                enablewidgets = false;
                endedscreen = false;
                (*it)->Enable(false);
            }

            CLuaGroup *lg = dynamic_cast<CLuaGroup *>(*it);
            endedscreen = lg->EndsScreen();
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
    
    TSTLVecSize size = m_WidgetRanges.size();
    if (size > 1)
    {
        m_pCounter->SetText(CreateText("%d/%d", m_CurSubScreen+1, SafeConvert<int>(size)));
        m_pCounter->Enable(true);
    }
    else
        m_pCounter->Enable(false);
    
    m_pTopBox->Enable((size > 1) || m_pTitle);
}

bool CInstallScreen::CheckWidgets()
{
    if (!m_pGroupBox)
        return true;
    
    const TChildList &list = m_pGroupBox->GetChildList();
    TChildList::const_iterator start, end;
    
    if (HasPrevWidgets())
        start = std::find(list.begin(), list.end(), m_WidgetRanges[m_CurSubScreen]);
    else
        start = list.begin();
    
    if (HasNextWidgets())
        end = std::find(list.begin(), list.end(), m_WidgetRanges[m_CurSubScreen+1]);
    else
        end = list.end();

    while (start != end)
    {
        CLuaGroup *w = dynamic_cast<CLuaGroup *>(*start);
        if (w && !w->CheckWidgets())
            return false;
        
        start++;
    }
    
    return true;
}

void CInstallScreen::ActivateSubScreen(TSTLVecSize screen)
{
    const TChildList &list = m_pGroupBox->GetChildList();
    TChildList::const_iterator start, end;
    
    for (start=list.begin(); start!=list.end(); start++)
        (*start)->Enable(false);
    
    start = std::find(list.begin(), list.end(), m_WidgetRanges[screen]);
    
    if ((m_WidgetRanges.size()-1) > screen)
        end = std::find(list.begin(), list.end(), m_WidgetRanges[screen+1]);
    else
        end = list.end();
    
    while (start != end)
    {
        if (VisibleWidget(*start))
            (*start)->Enable(true);
        start++;
    }
    
    m_WidgetRanges[screen]->ReqFocus();
    m_CurSubScreen = screen;
    UpdateCounter();
}

void CInstallScreen::CoreActivate()
{
    ResetWidgetRange();
    
    if (!m_WidgetRanges.empty())
        m_WidgetRanges[0]->ReqFocus();
    
    CBaseScreen::CoreActivate();
}

void CInstallScreen::CoreDraw()
{
    CBox::CoreDraw();

    if (m_bDirty && !m_WidgetRanges.empty() && m_pGroupBox)
    {
        m_bDirty = false;
        m_bCleaning = true;
        
        const TChildList &list = m_pGroupBox->GetChildList();
        NNCurses::CWidget *curstartw = m_WidgetRanges[m_CurSubScreen];
        
        ResetWidgetRange();
        
        const TSTLVecSize size = m_WidgetRanges.size();
        for (TSTLVecSize n=0; n<size; n++)
        {
            if ((n+1) < size)
            {
                TChildList::const_iterator start = std::find(list.begin(), list.end(), m_WidgetRanges[n]);
                if (start != list.end())
                {
                    TChildList::const_iterator end = std::find(start, list.end(), m_WidgetRanges[n+1]);
                    TChildList::const_iterator it = std::find(start, end, curstartw);
                    if (it != end)
                    {
                        ActivateSubScreen(n);
                        break;
                    }
                }
            }
            else
            {
                // last screen, assume it's there
                ActivateSubScreen(n);
                break;
            }
        }
        m_bCleaning = false;
    }
}

bool CInstallScreen::CoreHandleEvent(CWidget *emitter, int event)
{
    if (event == EVENT_REQUPDATE)
        m_bDirty = !m_bCleaning;
    
    return CBox::CoreHandleEvent(emitter, event);
}

bool CInstallScreen::HasPrevWidgets() const
{
    return (m_pGroupBox && !m_WidgetRanges.empty() && m_CurSubScreen);
}

bool CInstallScreen::HasNextWidgets() const
{
    return (m_pGroupBox && !m_WidgetRanges.empty() && (m_CurSubScreen < (m_WidgetRanges.size()-1)));
}

bool CInstallScreen::SubBack()
{
    if (!HasPrevWidgets())
        return false;
    
    ActivateSubScreen(m_CurSubScreen - 1);
    return true;
}

bool CInstallScreen::SubNext(bool check)
{
    if (check && !CheckWidgets())
        return true; // Widget check failed, so return true in order to stay at this screen
    
    if (!HasNextWidgets())
        return false;
    
    ActivateSubScreen(m_CurSubScreen + 1);
    return true;
}

void CInstallScreen::SubLast()
{
    if (m_WidgetRanges.size() > 1)
        ActivateSubScreen(m_WidgetRanges.size()-1);
}
