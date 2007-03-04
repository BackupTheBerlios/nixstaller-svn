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
#include "baseinstscreen.h"

// -------------------------------------
// Base Install Screen Class
// -------------------------------------

void CBaseScreen::SetLabel(const gchar *text)
{
    m_szOrigLabel = text;
    
    if (!m_pLabel)
    {
        m_pLabel = gtk_label_new(text);
        gtk_widget_show(m_pLabel);
        gtk_container_add(GTK_CONTAINER(m_pOwnerBox), m_pLabel);
    }
    else
        gtk_label_set(GTK_LABEL(m_pLabel), text);
}

void CBaseScreen::UpdateTranslations()
{
    if (m_pLabel)
        gtk_label_set(GTK_LABEL(m_pLabel), GetTranslation(m_szOrigLabel));
    
    UpdateTr();
}
