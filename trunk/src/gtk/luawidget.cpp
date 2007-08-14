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

#include "gtk.h"
#include "luawidget.h"

// -------------------------------------
// Base GTK Lua Widget Class
// -------------------------------------

CLuaWidget::CLuaWidget(void) : m_pTitle(NULL)
{
    m_pTitleBox = gtk_vbox_new(FALSE, 0);
    
    m_pBox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(m_pBox), m_pTitleBox);
}

void CLuaWidget::CoreSetTitle()
{
    if (!GetTitle().empty())
    {
        if (!m_pTitle)
        {
            m_pTitle = gtk_label_new(GetTranslation(GetTitle().c_str()));
            gtk_label_set_line_wrap(GTK_LABEL(m_pTitle), TRUE);
            gtk_widget_set_size_request(m_pTitle, MaxWidgetReqW(), -1);
            gtk_container_add(GTK_CONTAINER(m_pTitleBox), m_pTitle);
            
            gtk_widget_show_all(m_pTitleBox);
        }
        else
            gtk_label_set(GTK_LABEL(m_pTitle), GetTranslation(GetTitle().c_str()));
    }
}
