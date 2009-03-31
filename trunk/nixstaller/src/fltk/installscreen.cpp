/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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

#include "main/frontend/utils.h"
#include "fltk.h"
#include "installer.h"
#include "installscreen.h"
#include "luagroup.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

// -------------------------------------
// FLTK Install Screen Class
// -------------------------------------

CInstallScreen::CInstallScreen(const std::string &title, bool preview) : CBaseScreen(title, preview),
                                                                         m_CurSubScreen(0), m_iMaxHeight(0)
{
    // Size and positions are set by CInstaller when screen is activated
    m_pMainPack = new Fl_Pack(0, 0, 0, 0);
    m_pMainPack->type(Fl_Pack::VERTICAL);
    m_pMainPack->resizable(NULL);
    
    m_pCounter = new Fl_Box(0, 0, 0, 0);
    fl_font(m_pCounter->labelfont(), m_pCounter->labelsize());
    m_pCounter->size(0, fl_height());
    m_pCounter->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
    
    m_pWidgetGroup = new Fl_Group(0, 0, 0, 0);
    m_pWidgetGroup->resizable(NULL);
    
    m_pWidgetPack = new Fl_Pack(0, 0, 1, 0); // HACK: Dummy width, otherwise widgets won't shown up :)
    m_pWidgetPack->type(Fl_Pack::VERTICAL);
//     m_pWidgetPack->spacing(WidgetHSpacing());
    m_pWidgetPack->resizable(NULL);
    m_pWidgetPack->end();

    m_pWidgetGroup->end();
    
    m_pMainPack->end();
}

CBaseLuaGroup *CInstallScreen::CreateGroup(void)
{
    CLuaGroup *ret = new CLuaGroup(this);
    ret->GetGroup()->user_data(ret);
    ret->GetGroup()->hide();
    m_pWidgetPack->add(ret->GetGroup());
    return ret;
}

int CInstallScreen::GetWidgetSpacing()
{
    const int size = m_pWidgetPack->children();
    
    if (!size)
        return 0;
    
    int totalsize = 0;
    int start = m_WidgetRanges[m_CurSubScreen], end;
    
    if ((m_WidgetRanges.size()-1) > m_CurSubScreen)
        end = m_WidgetRanges[m_CurSubScreen+1];
    else
        end = size;
    
    while (start != end)
    {
        CLuaGroup *lg = GetGroup(m_pWidgetPack->child(start));
        if ((!lg || lg->IsEnabled()) && lg->GetGroup()->children())
            totalsize += m_pWidgetPack->child(start)->h();
        start++;
    }
    
    assert(totalsize <= m_iMaxHeight);
    
    return (m_iMaxHeight - totalsize) / size;
}

CLuaGroup *CInstallScreen::GetGroup(Fl_Widget *w)
{
    return static_cast<CLuaGroup *>(w->user_data());
}

int CInstallScreen::CheckTotalWidgetH(Fl_Widget *w)
{
    int ret = w->h();
    
    if (ret > m_iMaxHeight)
        throw Exceptions::CExOverflow("Not enough space for widget.");
    
    return ret;
}

void CInstallScreen::ResetWidgetRange()
{
    m_WidgetRanges.clear();
    m_CurSubScreen = 0;
    
    const int size = m_pWidgetPack->children(), spacing = MinWidgetSpacing();
    
    if (!size)
        return;
    
    m_WidgetRanges.push_back(0);
    
    int h = 0, drawwidgets = 0, totalh = 0;
    bool enablewidgets = true, endedscreen = false;
    
    for (int i=0; i<size; i++)
    {
        CLuaGroup *lg = GetGroup(m_pWidgetPack->child(i));
        Fl_Group *group = lg->GetGroup();
        
        if (!lg->IsEnabled() || !group->children())
        {
            group->hide();
            continue;
        }
        
        const int newh = CheckTotalWidgetH(group);

        if (!endedscreen && ((newh + h) <= m_iMaxHeight))
        {
            h += newh + spacing;
            if (enablewidgets)
            {
                drawwidgets++;
                totalh += newh;
                group->show();
            }
            else
                group->hide();
        }
        else
        {
            enablewidgets = false;
            endedscreen = false;
            m_WidgetRanges.push_back(i);
            h = newh;
            group->hide();
        }
        endedscreen = lg->EndsScreen();
    }
    
    int starth = totalh + (GetWidgetSpacing() * (drawwidgets-1));
    int y = m_pWidgetGroup->y() + ((m_iMaxHeight - starth) / 2); // Center
    m_pWidgetPack->resize(m_pWidgetPack->x(), y, m_pWidgetPack->w(), starth);
    
    UpdateCounter();
}

void CInstallScreen::UpdateCounter()
{
    TSTLVecSize size = m_WidgetRanges.size();
    if (size > 1)
        m_pCounter->label(CreateText("%d/%d", m_CurSubScreen+1, SafeConvert<int>(size)));
    else
        m_pCounter->label(NULL);
}

bool CInstallScreen::CheckWidgets()
{
    const int size = m_pWidgetPack->children();
    
    if (!size)
        return true;
    
    int start, end;
    if (HasPrevWidgets())
        start = m_WidgetRanges[m_CurSubScreen];
    else
        start = 0;
    
    if (HasNextWidgets())
        end = m_WidgetRanges[m_CurSubScreen+1];
    else
        end = size;
    
    bool ret = true;
    
    for (; start<end; start++)
    {
        CLuaGroup *group = GetGroup(m_pWidgetPack->child(start));
        if (!group->CheckWidgets())
        {
            ret = false;
            break;
        }
        
        // Need to recheck this everytime, as verify() functions from Lua may invalidate it
        if (HasNextWidgets())
            end = m_WidgetRanges[m_CurSubScreen+1];
        else
            end = size;
    }

    return ret;
}

void CInstallScreen::CoreActivate(void)
{
    ResetWidgetRange();
    m_pWidgetPack->spacing(GetWidgetSpacing());
    CBaseScreen::CoreActivate();
}

Fl_Group *CInstallScreen::GetGroup()
{
    return m_pMainPack;
}

void CInstallScreen::SetSize(int x, int y, int w, int h)
{
    m_pMainPack->resize(x, y, w, h);
    // We cannot use the height from a pack (m_pMainPack or m_pWidgetPack), because vertical packs will
    // auto adjust their height
    m_iMaxHeight = h - (m_pCounter->h() * 2);
    UpdateCounter();
    
    m_pWidgetGroup->resize(0, 0, w, m_iMaxHeight);

    const int size = m_pWidgetPack->children();
    for (int i=0; i<size; i++)
    {
        CLuaGroup *g = GetGroup(m_pWidgetPack->child(i));
        g->SetSize(w, m_iMaxHeight);
    }
    
    m_pWidgetPack->size(w, 0);
//     m_pWidgetPack->spacing(GetWidgetSpacing());
}

void CInstallScreen::ActivateSubScreen(TSTLVecSize screen)
{
    m_CurSubScreen = screen; // Need to set this before call GetWidgetSpacing()
    const int size = m_pWidgetPack->children(), spacing = GetWidgetSpacing();
    m_pWidgetPack->spacing(spacing);
    
    for (int i=0; i<size; i++)
        m_pWidgetPack->child(i)->hide();
    
    int start = m_WidgetRanges[screen], end;
    
    if ((m_WidgetRanges.size()-1) > screen)
        end = m_WidgetRanges[screen+1];
    else
        end = size;
    
    int h = 0;
    while (start != end)
    {
        CLuaGroup *lg = GetGroup(m_pWidgetPack->child(start));
        if ((!lg || lg->IsEnabled()) && lg->GetGroup()->children())
        {
            h += CheckTotalWidgetH(m_pWidgetPack->child(start)) + spacing;
            m_pWidgetPack->child(start)->show();
        }
        start++;
    }
    
    if (h)
    {
        UpdateCounter();
        int y = m_pWidgetGroup->y() + ((m_iMaxHeight - (h-spacing)) / 2); // Center
        m_pWidgetPack->resize(m_pWidgetPack->x(), y, m_pWidgetPack->w(), h);
    }
    
    UpdateCounter();
}

bool CInstallScreen::HasPrevWidgets() const
{
    return (!m_WidgetRanges.empty() && m_CurSubScreen);
}

bool CInstallScreen::HasNextWidgets() const
{
    return (!m_WidgetRanges.empty() && (m_CurSubScreen < (m_WidgetRanges.size()-1)));
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
    if (!m_WidgetRanges.empty())
        ActivateSubScreen(m_WidgetRanges.size()-1);
}

void CInstallScreen::UpdateSubScreens()
{
    if (m_WidgetRanges.empty())
        m_pWidgetPack->redraw(); // Only redraw
    else
    {
        Fl_Widget *group = m_pMainPack->child(m_WidgetRanges[m_CurSubScreen]);
        
        ResetWidgetRange();
        
        int ind = m_pWidgetPack->find(group);
        
        if (ind != m_pWidgetPack->children())
        {
            TSTLVecSize size = m_WidgetRanges.size();
            for (TSTLVecSize n=0; n<size; n++)
            {
                if (m_WidgetRanges[n] >= ind)
                {
                    ActivateSubScreen(n);
                    break;
                }
            }
        }
    }
}
