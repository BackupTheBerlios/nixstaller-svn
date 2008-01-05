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

#include "main/main.h"
#include "gtk.h"
#include "luacheckbox.h"

// -------------------------------------
// Lua Checkbox Class
// -------------------------------------

CLuaCheckbox::CLuaCheckbox(const char *desc, const TOptions &l) : CBaseLuaWidget(desc), CBaseLuaCheckbox(l)
{
    TSTLVecSize size = l.size(), n;
    
    for (n=0; n<size; n++)
    {
        m_Checkboxes.push_back(gtk_check_button_new_with_label(GetTranslation(l[n].c_str())));
        g_signal_connect(G_OBJECT(m_Checkboxes[n]), "toggled", G_CALLBACK(ToggleCB), this);
        gtk_widget_show(m_Checkboxes[n]);
        gtk_container_add(GTK_CONTAINER(GetBox()), m_Checkboxes[n]);
    }
}

bool CLuaCheckbox::Enabled(TSTLVecSize n)
{
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_Checkboxes[n])) == TRUE;
}

void CLuaCheckbox::Enable(TSTLVecSize n, bool b)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Checkboxes[n]), (b) ? TRUE : FALSE);
}

void CLuaCheckbox::CoreUpdateLanguage()
{
    TOptions &opts = GetOptions();
    TSTLVecSize size = opts.size(), n;
    
    for (n=0; n<size; n++)
        gtk_button_set_label(GTK_BUTTON(m_Checkboxes[n]), GetTranslation(opts[n].c_str()));
}

void CLuaCheckbox::CoreActivateWidget()
{
    if (m_Checkboxes.empty())
        return;
    
    gtk_widget_grab_focus(m_Checkboxes[0]);
}

void CLuaCheckbox::ToggleCB(GtkToggleButton *togglebutton, gpointer data)
{
    CLuaCheckbox *box = static_cast<CLuaCheckbox *>(data);
    box->SafeLuaDataChanged();
}
