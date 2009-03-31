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

#include "gtk.h"
#include "luaprogressbar.h"

// -------------------------------------
// Lua Progress Bar Class
// -------------------------------------

CLuaProgressBar::CLuaProgressBar(const char *desc) : CBaseLuaWidget(desc)
{
    m_pProgressBar = gtk_progress_bar_new();
    gtk_widget_show(m_pProgressBar);
    gtk_container_add(GTK_CONTAINER(GetBox()), m_pProgressBar);
}

void CLuaProgressBar::SetProgress(float n)
{
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(m_pProgressBar), static_cast<double>(n) / 100.0);
}

