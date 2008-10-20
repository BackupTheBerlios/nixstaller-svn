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
#include "installscreen.h"
#include "luawidget.h"

// -------------------------------------
// Base GTK Lua Widget Class
// -------------------------------------

CLuaWidget::CLuaWidget() : m_pInstallScreen(NULL)
{
    m_pBox = gtk_vbox_new(FALSE, 0);
    
    m_pTitle = gtk_label_new(NULL);
    gtk_label_set_line_wrap(GTK_LABEL(m_pTitle), TRUE);
    gtk_misc_set_alignment(GTK_MISC(m_pTitle), 0.0f, 0.0f);
    gtk_box_pack_start(GTK_BOX(m_pBox), m_pTitle, TRUE, TRUE, 4);
}

void CLuaWidget::CoreSetTitle()
{
    if (!GetTitle().empty())
    {
        gtk_label_set(GTK_LABEL(m_pTitle), GetTranslation(GetTitle().c_str()));
        SetMaxWidth(m_iMaxWidth); // Need to update this with new text
        gtk_widget_show(m_pTitle);
        UpdateScreenLayout();
    }
}

void CLuaWidget::SafeLuaDataChanged()
{
    try
    {
        LuaDataChanged();
    }
    catch(Exceptions::CException &)
    {
        HandleError();
    }
}

void CLuaWidget::UpdateScreenLayout()
{
    if (m_pInstallScreen)
        m_pInstallScreen->UpdateSubScreens();
}

void CLuaWidget::SetMaxWidth(int w)
{
    m_iMaxWidth = w;
    gtk_widget_set_size_request(m_pTitle, w, -1);
    CoreUpdateMaxWidth(w);
}
