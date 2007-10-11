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

#include "main/main.h"
#include "gtk.h"
#include "luaradiobutton.h"

// -------------------------------------
// Lua Radio Button Class
// -------------------------------------

CLuaRadioButton::CLuaRadioButton(const char *desc, const TOptions &l) : CBaseLuaWidget(desc), CBaseLuaRadioButton(l)
{
    TSTLVecSize size = l.size(), n;
    
    for (n=0; n<size; n++)
    {
        if (m_RadioButtons.empty())
            m_RadioButtons.push_back(gtk_radio_button_new_with_label(NULL, GetTranslation(l[n].c_str())));
        else
            m_RadioButtons.push_back(gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(m_RadioButtons[0]),
                                     GetTranslation(l[n].c_str())));
        g_signal_connect(G_OBJECT(m_RadioButtons[n]), "toggled", G_CALLBACK(ToggleCB), this);
        gtk_widget_show(m_RadioButtons[n]);
        gtk_container_add(GTK_CONTAINER(GetBox()), m_RadioButtons[n]);
    }
}

const char *CLuaRadioButton::EnabledButton()
{
    TOptions &opts = GetOptions();
    TSTLVecSize size = opts.size(), n;
    
    for (n=0; n<size; n++)
    {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_RadioButtons[n])))
            return opts[n].c_str();
    }
    
    return NULL;
}

void CLuaRadioButton::Enable(TSTLVecSize n)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RadioButtons[n]), TRUE);
}

void CLuaRadioButton::CoreUpdateLanguage()
{
    TOptions &opts = GetOptions();
    TSTLVecSize size = opts.size(), n;
    
    for (n=0; n<size; n++)
        gtk_button_set_label(GTK_BUTTON(m_RadioButtons[n]), GetTranslation(opts[n].c_str()));
}

void CLuaRadioButton::CoreActivateWidget()
{
    if (m_RadioButtons.empty())
        return;
    
    gtk_widget_grab_focus(m_RadioButtons[0]);
}

void CLuaRadioButton::ToggleCB(GtkToggleButton *togglebutton, gpointer data)
{
    CLuaRadioButton *radio = static_cast<CLuaRadioButton *>(data);
    radio->SafeLuaDataChanged();
}
