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

#include "main/main.h"
#include "fltk.h"
#include "luagroup.h"
#include "luawidget.h"
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>
#include <FL/fl_draw.H>

// -------------------------------------
// Base FLTK Lua Widget Class
// -------------------------------------

CLuaWidget::CLuaWidget(void)
{
    m_pMainPack = new Fl_Pack(0, 0, 0, 0); // Widgets need to set this
    m_pMainPack->type(Fl_Pack::VERTICAL);
    m_pMainPack->resizable(NULL);
    
    m_pTitle = new Fl_Box(0, 0, 0, 0);
    m_pTitle->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_WRAP);
    
    m_pMainPack->end();
}

void CLuaWidget::CoreSetTitle()
{
    if (!GetTitle().empty())
        m_pTitle->label(MakeTranslation(GetTitle()));
    RedrawWidgetRecursive(GetGroup());
    GetParent()->UpdateLayout();
}

void CLuaWidget::CoreActivateWidget()
{
    Fl::focus(m_pMainPack);
}

void CLuaWidget::UpdateVisible(bool v)
{
    if (GetParent()) // Parent may be NULL when called from constructor
    {
        RedrawWidgetRecursive(GetGroup());
        GetParent()->UpdateLayout();
    }
}

Fl_Group *CLuaWidget::GetGroup()
{
    return m_pMainPack;
}

int CLuaWidget::RequestWidth()
{
    const int minw = (!GetTitle().empty()) ? 150 : 0;
    return std::max(minw, CoreRequestWidth());
}

int CLuaWidget::RequestHeight(int maxw)
{
    int ret = 0;
    
    if (!GetTitle().empty())
    {
        int w = maxw, h = 0;
        fl_font(m_pMainPack->labelfont(), m_pMainPack->labelsize());
        fl_measure(GetTitle().c_str(), w, h);
        ret = h;
    }
    
    ret += CoreRequestHeight(maxw);
    
    return ret;
}

void CLuaWidget::SetSize(int w, int h)
{
    m_pMainPack->size(w, h);

    if (!GetTitle().empty())
    {
        int tw = w, th = 0;
        fl_font(m_pMainPack->labelfont(), m_pMainPack->labelsize());
        fl_measure(GetTitle().c_str(), tw, th);
        
        if ((tw != m_pTitle->w()) || (th != m_pTitle->h()))
            m_pTitle->size(tw, th);
    }
    
    UpdateSize();
}
