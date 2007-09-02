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
        m_pTitle->label(GetTranslation(GetTitle().c_str()));
}

Fl_Group *CLuaWidget::GetGroup()
{
    return m_pMainPack;
}

void CLuaWidget::SetSize(int maxw, int maxh)
{
    if (!GetTitle().empty())
    {
        int w = maxw, h = 0;
        fl_font(m_pMainPack->labelfont(), m_pMainPack->labelsize());
        fl_measure(GetTitle().c_str(), w, h);
        w = maxw;
        
        if ((w > m_pTitle->w()) || (h > m_pTitle->h()))
        {
            int mainw = std::max(w, m_pMainPack->w());
            int mainh = std::max(h, m_pMainPack->h());
            
            if ((mainw != m_pMainPack->w()) || (mainh != m_pMainPack->h()))
                m_pMainPack->size(mainw, mainh);
            
            m_pTitle->size(w, h);
        }
    }
    
    CoreSetSize(maxw, maxh);
}
