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

CInstallScreen::CInstallScreen(const std::string &title) : CBaseScreen(title), m_WidgetRange(NULL, NULL)
{
    // Size and positions are set by CInstaller when screen is activated
    m_pMainPack = new Fl_Pack(0, 0, 0, 0);
    m_pMainPack->type(Fl_Pack::VERTICAL);
    
    // We let the counter have a static height, since this makes resizing for the widget pack easier 
    // (as otherwise the size would depend on each other)
    m_pCounter = new Fl_Box(0, 0, 0, 0);
    fl_font(m_pCounter->labelfont(), m_pCounter->labelsize());
    m_pCounter->size(0, fl_height());
    m_pCounter->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
    m_pCounter->hide();
    
    m_pWidgetPack = new Fl_Pack(0, 0, 1, 0); // HACK: Dummy width, otherwise widgets won't shown up :)
    m_pWidgetPack->type(Fl_Pack::VERTICAL);
    m_pWidgetPack->spacing(WidgetHSpacing());
    m_pWidgetPack->resizable(NULL);
    m_pWidgetPack->end();
    
    m_pMainPack->end();
}

CBaseLuaGroup *CInstallScreen::CreateGroup(void)
{
    CLuaGroup *ret = new CLuaGroup(m_pWidgetPack->w());
    ret->GetGroup()->user_data(ret);
    ret->GetGroup()->hide();
    m_pWidgetPack->add(ret->GetGroup());
    return ret;
}

int CInstallScreen::MaxScreenHeight(void)
{
    return m_pWidgetPack->h();
}

CLuaGroup *CInstallScreen::GetGroup(Fl_Widget *w)
{
    return static_cast<CLuaGroup *>(w->user_data());
}

int CInstallScreen::CheckTotalWidgetH(Fl_Widget *w)
{
    int ret = w->h() + WidgetHSpacing();
    
    if (ret > m_pWidgetPack->h())
        throw Exceptions::CExOverflow("Not enough space for widget.");
    
    return ret;
}

void CInstallScreen::ResetWidgetRange()
{
    m_WidgetRange.first = m_WidgetRange.second = NULL;
    
    const int size = m_pWidgetPack->children();
    
    if (!size)
        return;
    
    int h = 0;
    for (int i=0; i<size; i++)
    {
        Fl_Group *group = GetGroup(m_pWidgetPack->child(i))->GetGroup();
        if (!group->children())
            continue;
        
        h += CheckTotalWidgetH(group);

        if (h <= MaxScreenHeight())
        {
            group->show();
            m_WidgetRange.second = group;
            continue;
        }
        
        group->hide();
    }
    
    UpdateCounter();
}

void CInstallScreen::UpdateCounter()
{
    const int size = m_pWidgetPack->children();
    int count = 1, current = 1, h = m_pCounter->h();
    
    for (int i=0; i<size; i++)
    {
        Fl_Group *group = GetGroup(m_pWidgetPack->child(i))->GetGroup();
        if (!group->children())
            continue;

        const int wh = CheckTotalWidgetH(group);
        
        if ((h + wh) > MaxScreenHeight())
        {
            h = 0;
            count++;
        }
        
        h += wh;

        if (group == m_WidgetRange.first)
            current = count;
    }
    
    if (count > 1)
    {
        m_pCounter->label(CreateText("%d/%d", current, count));
        m_pCounter->show();
    }
    else
        m_pCounter->hide();
}

void CInstallScreen::CoreActivate(void)
{
    ResetWidgetRange();
    CBaseScreen::CoreActivate();
}

Fl_Group *CInstallScreen::GetGroup()
{
    return m_pMainPack;
}

void CInstallScreen::SetSize(int x, int y, int w, int h)
{
    m_pMainPack->resize(x, y, w, h);
    m_pWidgetPack->size(w, h-m_pCounter->h());
    UpdateCounter(); // This needs to be done after resizing the widget group
}
