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
#include "luawidget.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

// -------------------------------------
// Base FLTK Lua Widget Class
// -------------------------------------

CLuaWidget::CLuaWidget(void)
{
    m_pMainGroup = new Fl_Group(0, 0, 0, 0); // Widgets need to set this
    m_pMainGroup->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_WRAP);
    m_pMainGroup->end();
}

void CLuaWidget::CoreSetTitle()
{
    if (!GetTitle().empty())
        m_pMainGroup->label(GetTranslation(GetTitle().c_str()));
}

void CLuaWidget::LabelSize(int &w, int &h) const
{
    assert(m_pMainGroup->parent());
    w = m_pMainGroup->parent()->w();
    h = 0;
    fl_font(m_pMainGroup->labelfont(), m_pMainGroup->labelsize());
    fl_measure(GetTitle().c_str(), w, h);
}
