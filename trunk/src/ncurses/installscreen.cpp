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
                                                           m_WidgetRange(NULL, NULL)
{
    NNCurses::CBox *box = new NNCurses::CBox(HORIZONTAL, false);
    box->AddWidget(m_pTitle = new NNCurses::CLabel(title));
    box->EndPack(m_pCounter = new NNCurses::CLabel(""), false, false, 0, 1);
    m_pCounter->SetDFColors(COLOR_RED, COLOR_BLUE);
    StartPack(box, false, false, 0, 0);
}

CBaseLuaGroup *CInstallScreen::CreateGroup()
{
    CLuaGroup *ret = new CLuaGroup();
    StartPack(ret, true, true, 0, 1);
    m_LuaGroups.push_back(ret);
    return ret;
}

void CInstallScreen::CoreUpdateLanguage(void)
{
    m_pTitle->SetText(GetTranslation(GetTitle()));
}

void CInstallScreen::ResetWidgetRange()
{
    m_WidgetRange.first = m_WidgetRange.second = NULL;
    
/*    if (!GetWin())
        return; // Not initialized yet*/
    
    int h = StartingHeight();
    
    for (TChildList::const_iterator it=m_LuaGroups.begin(); it!=m_LuaGroups.end(); it++)
    {
        if (h < MaxScreenHeight())
        {
            h += GetTotalWidgetH(*it);
            
            if (h < MaxScreenHeight())
            {
                (*it)->Enable(true);
                m_WidgetRange.second = *it;
                continue;
            }
        }
        
        (*it)->Enable(false);
    }
    
    UpdateCounter();
}

int CInstallScreen::StartingHeight()
{
    return m_pTitle->Y() + m_pTitle->Height();
}

void CInstallScreen::UpdateCounter()
{
    int count = 1, current = 1, h = StartingHeight();
    
    for (TChildList::const_iterator it=m_LuaGroups.begin(); it!=m_LuaGroups.end(); it++)
    {
        h += GetTotalWidgetH(*it);
        
        if (h >= MaxScreenHeight())
        {
            count++;
            h = 0;
        }
        
        if (*it == m_WidgetRange.first)
            current = count;
    }
    
    if (count == 1)
        m_pCounter->Clear();
    else
        m_pCounter->SetText(CreateText("%d/%d", current, count));
}

bool CInstallScreen::SubBack()
{
    if (HasPrevWidgets())
    {
        TChildList::reverse_iterator it = m_LuaGroups.rbegin();
        
        for (; it!=m_LuaGroups.rend(); it++)
            (*it)->Enable(false);
        
        int h = StartingHeight();
        it = std::find(m_LuaGroups.rbegin(), m_LuaGroups.rend(), m_WidgetRange.first);
        m_WidgetRange.first = m_WidgetRange.second = NULL;
        
        if (it != m_LuaGroups.rend())
        {
            while ((*it != m_LuaGroups.front()) && (h < MaxScreenHeight()))
            {
                it++;
                h += GetTotalWidgetH(*it);
                
                if (h < MaxScreenHeight())
                {
                    (*it)->Enable(true);
                    m_WidgetRange.first = *it;
                }
                
                if (!m_WidgetRange.second)
                    m_WidgetRange.second = *it;
            }
        }
        
        if (h != StartingHeight())
        {
            UpdateCounter();
            return true;
        }
    }
    
    return false;
}

bool CInstallScreen::SubNext()
{
    if (HasNextWidgets())
    {
        TChildList::const_iterator it = m_LuaGroups.begin();
        
        for (; it!=m_LuaGroups.end(); it++)
            (*it)->Enable(false);
        
        int h = StartingHeight();
        it = std::find(m_LuaGroups.begin(), m_LuaGroups.end(), m_WidgetRange.second);
        m_WidgetRange.first = m_WidgetRange.second = NULL;
        
        if (it != m_LuaGroups.end())
        {
            while ((*it != m_LuaGroups.back()) && (h < MaxScreenHeight()))
            {
                it++;
                h += GetTotalWidgetH(*it);
                
                if (h < MaxScreenHeight())
                {
                    (*it)->Enable(true);
                    m_WidgetRange.second = *it;
                }
                
                if (!m_WidgetRange.first)
                    m_WidgetRange.first = *it;
            }
        }
        
        if (h != StartingHeight())
        {
            UpdateCounter();
            SetNextFocWidget(false); // Focus first widget
            return true;
        }
    }
    
    return false;
}

int CInstallScreen::CoreRequestHeight()
{
    return std::min(NNCurses::CBox::CoreRequestHeight(), MaxScreenHeight());
}
