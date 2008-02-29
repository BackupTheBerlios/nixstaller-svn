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
#include "main/main.h"
#include "luaprogressdialog.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Window.H>

// -------------------------------------
// FLTK Lua Progress Dialog Class
// -------------------------------------

CLuaProgressDialog::CLuaProgressDialog(const CBaseLuaProgressDialog::TStepList &l, int r) : CBaseLuaProgressDialog(l, r)
{
    const int windoww = 500, windowh = 140, widgetw = windoww - 20, x = 10;
    m_pDialog = new Fl_Window(windoww, windowh);
    m_pDialog->set_modal();
    m_pDialog->begin();
    
    m_pTitle = new Fl_Box(x, 0, widgetw, WidgetHeight(), MakeTranslation(GetStepList()[0]));
    
    m_pProgBar = new Fl_Progress(x, 0, widgetw, WidgetHeight());
    m_pProgBar->minimum(0.0f);
    m_pProgBar->maximum(100.0f);
    
    m_pSecTitle = new Fl_Box(x, 0, widgetw, WidgetHeight());
    m_pSecTitle->hide();
    
    m_pSecProgBar = new Fl_Progress(x, 0, widgetw, WidgetHeight());
    m_pSecProgBar->minimum(0.0f);
    m_pSecProgBar->maximum(100.0f);
    m_pSecProgBar->hide();
    
    Fl_Button *button = new Fl_Button(0, windowh - WidgetHeight() - 5, 0, WidgetHeight(), GetTranslation("Cancel"));
    SetButtonWidth(button);
    button->position((windoww - button->w()) / 2, button->y());
    button->callback(CancelCB, this);
    
    UpdateWidgets();
    
    m_pDialog->end();
    m_pDialog->show();
}

CLuaProgressDialog::~CLuaProgressDialog()
{
    delete m_pDialog;
}

void CLuaProgressDialog::UpdateWidgets()
{
    // totalh uses +1 widget because the cancel button
    if (m_pSecTitle->visible())
    {
        const int totalh = 5 * (WidgetHeight() + 5);
        int y = (m_pDialog->h() - totalh) / 2;
        
        m_pTitle->position(m_pTitle->x(), y);
        
        y += WidgetHeight() + 5;
        m_pProgBar->position(m_pProgBar->x(), y);

        y += WidgetHeight() + 5;
        m_pSecTitle->position(m_pSecTitle->x(), y);
        
        y += WidgetHeight() + 5;
        m_pSecProgBar->position(m_pSecProgBar->x(), y);
    }
    else
    {
        const int totalh = 3 * (WidgetHeight() + 5);
        int y = (m_pDialog->h() - totalh) / 2;
        
        m_pTitle->position(m_pTitle->x(), y);
        
        y += WidgetHeight() + 5;
        m_pProgBar->position(m_pProgBar->x(), y);
    }
}

void CLuaProgressDialog::CoreSetNextStep()
{
    const TStepList::size_type step = GetCurrentStep();
    m_pTitle->label(MakeTranslation(GetStepList()[step]));
    
    float n = static_cast<double>(step+1) / static_cast<double>(GetStepList().size()) * 100.0f;
    m_pProgBar->value(n);
}

void CLuaProgressDialog::CoreSetProgress(int progress)
{
    m_pProgBar->value(static_cast<float>(progress));
}

void CLuaProgressDialog::CoreEnableSecProgBar(bool enable)
{
    if (enable)
    {
        m_pSecTitle->show();
        m_pSecProgBar->show();
    }
    else
    {
        m_pSecTitle->hide();
        m_pSecProgBar->hide();
    }
    
    UpdateWidgets();
}

void CLuaProgressDialog::CoreSetSecTitle(const char *title)
{
    m_pSecTitle->label(GetTranslation(title));
}

void CLuaProgressDialog::CoreSetSecProgress(int progress)
{
    m_pSecProgBar->value(static_cast<float>(progress));
}

void CLuaProgressDialog::CancelCB(Fl_Widget *w, void *p)
{
    CLuaProgressDialog *dialog = static_cast<CLuaProgressDialog *>(p);
    dialog->SetCancelled();
}
