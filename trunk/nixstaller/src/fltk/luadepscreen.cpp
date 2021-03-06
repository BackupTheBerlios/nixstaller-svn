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
#include "luadepscreen.h"
#include "FL/Fl.H"
#include "FL/Fl_Box.H"
#include "FL/Fl_Browser.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Pack.H"
#include "FL/Fl_Window.H"
#include <FL/fl_draw.H>

// -------------------------------------
// FLTK Lua Dependency Screen Class
// -------------------------------------

CLuaDepScreen::CLuaDepScreen(CInstaller *inst, int f) : CBaseLuaDepScreen(f), m_pInstaller(inst), m_bClose(false)
{
    const int windoww = 500, windowh = 350, x = 20, widgetw = windoww - x - 20;
    m_pDialog = new Fl_Window(windoww, windowh);
    m_pDialog->callback(CInstaller::CancelCB, m_pInstaller);
    m_pDialog->set_modal();
    m_pDialog->begin();
    
    Fl_Box *title = new Fl_Box(x, 20, widgetw, 0, GetTitle());
    fl_font(title->labelfont(), title->labelsize());
    int w = widgetw, h;
    fl_measure(GetTitle(), w, h);
    title->size(title->w(), h);
    title->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_WRAP);
    
    int y = title->y() + title->h() + 20;
    m_pListBox = new Fl_Browser(x, y, widgetw, windowh - y - 60);
    
    y = m_pListBox->y() + m_pListBox->h() + 20;
    const int spacing = 20;
    Fl_Pack *bpack = new Fl_Pack(x, y, widgetw, 25);
    bpack->type(Fl_Pack::HORIZONTAL);
    bpack->spacing(spacing);

    Fl_Button *cancelb = new Fl_Button(0, 0, 0, 0, GetTranslation("Cancel"));
    cancelb->callback(CInstaller::CancelCB, m_pInstaller);
    SetButtonWidth(cancelb);
    
    Fl_Button *refreshb = new Fl_Button(0, 0, 0, 0, GetTranslation("Refresh"));
    refreshb->callback(RefreshCB, this);
    SetButtonWidth(refreshb);
    
    Fl_Button *ignoreb = new Fl_Button(0, 0, 0, 0, GetTranslation("Ignore"));
    ignoreb->callback(IgnoreCB, this);
    SetButtonWidth(ignoreb);
    
    bpack->end();
    w = cancelb->w() + refreshb->w() + ignoreb->w() + 2 * spacing;
    bpack->position((windoww - w) / 2, y);
    
    m_pDialog->end();
}

CLuaDepScreen::~CLuaDepScreen()
{
    delete m_pDialog;
}

void CLuaDepScreen::AddText(const std::string &s)
{
    TSTLStrSize start = 0, length = s.length();
    while (start < length)
    {
        TSTLStrSize ind = s.find("\n", start);
        m_pListBox->add(s.substr(start, (ind-start)).c_str());
        if (ind == std::string::npos)
            break;
        start = ind + 1;
    }
}

void CLuaDepScreen::CoreUpdateList()
{
    if (!m_pDialog->visible())
        m_pDialog->show();
        
    m_pListBox->clear();
    
    for (TDepList::const_iterator it=GetDepList().begin(); it!=GetDepList().end(); it++)
    {
        AddText("@m" + GetTranslation(it->name));
        if (!it->description.empty())
            AddText(GetTranslation(std::string("Description")) + ": " + GetTranslation(it->description));
        AddText(GetTranslation(std::string("Problem")) + ": " + GetTranslation(it->problem));
        AddText("");
    }
    
    if (GetDepList().empty())
        m_bClose = true;
}

void CLuaDepScreen::CoreRun()
{
    while ((Fl::wait(1.0f) > -1) && !m_bClose)
        ;
}

void CLuaDepScreen::RefreshCB(Fl_Widget *w, void *p)
{
    CLuaDepScreen *screen = static_cast<CLuaDepScreen *>(p);
    screen->Refresh();
}

void CLuaDepScreen::IgnoreCB(Fl_Widget *w, void *p)
{
    CLuaDepScreen *screen = static_cast<CLuaDepScreen *>(p);
    screen->m_bClose = true;
}
