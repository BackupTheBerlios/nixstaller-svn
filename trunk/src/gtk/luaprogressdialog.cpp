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

#include "gtk.h"
#include "luaprogressdialog.h"

// -------------------------------------
// GTK Lua Progress Dialog Class
// -------------------------------------

CLuaProgressDialog::CLuaProgressDialog(GtkWidget *parent, const CBaseLuaProgressDialog::TStepList &l,
                                       int r) : CBaseLuaProgressDialog(l, r)
{
    const int windoww = 500, windowh = 100;
    
    m_pDialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_transient_for(GTK_WINDOW(m_pDialog), GTK_WINDOW(parent));
    gtk_window_set_modal(GTK_WINDOW(m_pDialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(m_pDialog), windoww, windowh);
    gtk_window_set_position(GTK_WINDOW(m_pDialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show(GTK_WIDGET(m_pDialog));
    
    GtkWidget *vbox = gtk_vbox_new(FALSE, 15);
    gtk_widget_show(GTK_WIDGET(vbox));
    gtk_container_add(GTK_CONTAINER(m_pDialog), vbox);
    
    m_pTitle = gtk_label_new(l[0].c_str());
    gtk_widget_show(GTK_WIDGET(m_pTitle));
    gtk_box_pack_start(GTK_BOX(vbox), m_pTitle, FALSE, FALSE, 0);
    
    m_pProgBar = gtk_progress_bar_new();
    gtk_widget_show(GTK_WIDGET(m_pProgBar));
    gtk_box_pack_start(GTK_BOX(vbox), m_pProgBar, FALSE, FALSE, 0);
    
    m_pSecTitle = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(vbox), m_pSecTitle, FALSE, FALSE, 0);
    
    m_pSecProgBar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), m_pSecProgBar, FALSE, FALSE, 0);
}

void CLuaProgressDialog::CoreSetNextStep()
{
    VerifyRunning();
    
    const TStepList::size_type step = GetCurrentStep();
    gtk_label_set_text(GTK_LABEL(m_pTitle), GetStepList()[step].c_str());
    
    float n = static_cast<double>(step+1) / static_cast<double>(GetStepList().size());
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(m_pProgBar), n);
}

void CLuaProgressDialog::CoreEnableSecProgBar(bool enable)
{
    VerifyRunning();
    if (enable)
    {
        gtk_widget_show(GTK_WIDGET(m_pSecTitle));
        gtk_widget_show(GTK_WIDGET(m_pSecProgBar));
    }
    else
    {
        gtk_widget_hide(GTK_WIDGET(m_pSecTitle));
        gtk_widget_hide(GTK_WIDGET(m_pSecProgBar));
    }
}

void CLuaProgressDialog::CoreSetSecTitle(const char *title)
{
    VerifyRunning();
    gtk_label_set_text(GTK_LABEL(m_pSecTitle), title);
}

void CLuaProgressDialog::CoreSetSecProgress(int progress)
{
    VerifyRunning();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(m_pSecProgBar), static_cast<double>(progress) / 100.0);
}

void CLuaProgressDialog::CoreRun()
{
    m_bRunning = true;
    gtk_main_iteration_do(FALSE);
    CallFunction();
    gtk_widget_destroy(m_pDialog);
    m_bRunning = false;
}

void CLuaProgressDialog::VerifyRunning()
{
    if (!m_bRunning)
        luaL_error(NLua::LuaState, "Error: function called when progress dialog is not running.\n");
}
