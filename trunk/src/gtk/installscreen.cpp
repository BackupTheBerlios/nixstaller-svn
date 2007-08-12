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

#include "gtk.h"
#include "installer.h"
#include "installscreen.h"

// -------------------------------------
// GTK Install Screen Class
// -------------------------------------

CInstallScreen::CInstallScreen(const std::string &title) : CBaseScreen(title),
                               m_pTitle(NULL), m_pGroupBox(NULL), m_pNextSubScreen(NULL)
{
    m_pMainBox = gtk_vbox_new(FALSE, 0);
    
    m_pTopBox = gtk_hbox_new(FALSE, 0);
    
    if (!title.empty())
    {
        m_pTitle = gtk_label_new(title.c_str());
        gtk_widget_show(m_pTitle);
        gtk_container_add(GTK_CONTAINER(m_pTopBox), m_pTitle);
    }
    
    m_pCounter = gtk_label_new("");
    gtk_box_pack_end(GTK_BOX(m_pTopBox), m_pCounter, FALSE, FALSE, 10);
    
    if (m_pTitle)
        gtk_widget_show(m_pTopBox);
    
    gtk_container_add(GTK_CONTAINER(m_pMainBox), m_pTopBox);
}

void CInstallScreen::CoreUpdateLanguage(void)
{
    if (m_pTitle)
        gtk_label_set(GTK_LABEL(m_pTitle), GetTranslation(GetTitle().c_str()));
}

