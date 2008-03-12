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

#include "fltk.h"
#include "installer.h"
#include "luadepscreen.h"
#include "FL/Fl.H"
#include "FL/Fl_Browser.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Pack.H"
#include "FL/Fl_Window.H"

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
    
    m_pListBox = new Fl_Browser(x, 60, widgetw, windowh - 120, "GetTitle()\nhey\nhoh");
    m_pListBox->align(FL_ALIGN_TOP);
    
    const int y = m_pListBox->y() + m_pListBox->h() + 20, spacing = 20;
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
    int w = cancelb->w() + refreshb->w() + ignoreb->w() + 2 * spacing;
    bpack->position((windoww - w) / 2, y);
    
    m_pDialog->end();
    m_pDialog->show();
}

CLuaDepScreen::~CLuaDepScreen()
{
    delete m_pDialog;
}

void CLuaDepScreen::CoreUpdateList()
{
    m_pListBox->clear();
    
    for (TDepList::const_iterator it=GetDepList().begin(); it!=GetDepList().end(); it++)
    {
        m_pListBox->add(CreateText("@m%s\n", GetTranslation(it->name).c_str()));
        m_pListBox->add(CreateText("%s: %s\n", GetTranslation("Description"),
                        GetTranslation(it->description.c_str())));
        m_pListBox->add(CreateText("%s: %s\n", GetTranslation("Problem"),
                        GetTranslation(it->problem).c_str()));
        m_pListBox->add("\n");
    }
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
    if (screen->GetDepList().empty())
        screen->m_bClose = true;
}

void CLuaDepScreen::IgnoreCB(Fl_Widget *w, void *p)
{
    CLuaDepScreen *screen = static_cast<CLuaDepScreen *>(p);
    screen->m_bClose = true;
}
