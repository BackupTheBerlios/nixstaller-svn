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
    
    m_pCounter = new Fl_Box(0, 0, 0, 0);
    m_pCounter->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
    
    m_pWidgetGroup = new Fl_Group(0, 0, 1, 0); // HACK: Dummy width, otherwise widgets won't shown up :)
    m_pWidgetGroup->resizable(NULL);
    m_pWidgetGroup->end();
    
    m_pMainPack->end();
}

CBaseLuaGroup *CInstallScreen::CreateGroup(void)
{
    CLuaGroup *ret = new CLuaGroup(m_pWidgetGroup->w());
    ret->GetGroup()->user_data(ret);
    ret->GetGroup()->hide();
    m_pWidgetGroup->add(ret->GetGroup());
    return ret;
}

CLuaGroup *CInstallScreen::GetGroup(Fl_Widget *w)
{
    return static_cast<CLuaGroup *>(w->user_data());
}

int CInstallScreen::CheckTotalWidgetH(Fl_Widget *w)
{
    int ret = w->h() + WidgetHSpacing();
    
    if (ret > m_pWidgetGroup->h())
        throw Exceptions::CExOverflow("Not enough space for widget.");
    
    return ret;
}

void CInstallScreen::ResetWidgetRange()
{
    m_WidgetRange.first = m_WidgetRange.second = NULL;
    
    const int size = m_pWidgetGroup->children();
    
    if (!size)
        return;
    
    int h = 0;
    for (int i=0; i<size; i++)
    {
        Fl_Group *group = GetGroup(m_pWidgetGroup->child(i))->GetGroup();
        if (!group->children())
            continue;
        
        h += CheckTotalWidgetH(group);

        if (h <= m_pWidgetGroup->h())
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
    int w = 0, h = 0;
    fl_font(m_pCounter->labelfont(), m_pCounter->labelsize());
    fl_measure("(1/1)", w, h);
    m_pCounter->resize(m_pMainPack->w() - w, 0, w, h);
    m_pCounter->label("(1/1)");
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
    UpdateCounter();
    m_pWidgetGroup->size(w, h-m_pCounter->h());
}
