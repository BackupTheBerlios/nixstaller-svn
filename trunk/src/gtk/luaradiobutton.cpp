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

#include "main/main.h"
#include "gtk.h"
#include "luaradiobutton.h"

// -------------------------------------
// Lua Radio Button Class
// -------------------------------------

CLuaRadioButton::CLuaRadioButton(const char *desc, const TOptions &l,
                                 TSTLVecSize e) : CBaseLuaWidget(desc), CBaseLuaRadioButton(l), m_bInit(true)
{
    TSTLVecSize size = l.size(), n;
    
    for (n=0; n<size; n++)
    {
        if (m_RadioButtons.empty())
            m_RadioButtons.push_back(gtk_radio_button_new_with_label(NULL, GetTranslation(l[n].c_str())));
        else
            m_RadioButtons.push_back(gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(m_RadioButtons[0]),
                                     GetTranslation(l[n].c_str())));
        if (n == e)
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RadioButtons[n]), TRUE);
        
        g_signal_connect(G_OBJECT(m_RadioButtons[n]), "toggled", G_CALLBACK(ToggleCB), this);
        gtk_widget_show(m_RadioButtons[n]);
        gtk_container_add(GTK_CONTAINER(GetBox()), m_RadioButtons[n]);
    }
    
    m_bInit = false;
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

void CLuaRadioButton::AddButton(const std::string &label, TSTLVecSize n)
{
    if (m_RadioButtons.empty())
        m_RadioButtons.push_back(gtk_radio_button_new_with_label(NULL, GetTranslation(label.c_str())));
    else
    {
        GtkWidget *button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(m_RadioButtons[0]),
                            GetTranslation(label.c_str()));
        
        if (n >= GetOptions().size())
            m_RadioButtons.push_back(button);
        else
            m_RadioButtons.insert(m_RadioButtons.begin() + n, button);
    }
    
    g_signal_connect(G_OBJECT(m_RadioButtons[n]), "toggled", G_CALLBACK(ToggleCB), this);
    gtk_widget_show(m_RadioButtons[n]);
    gtk_container_add(GTK_CONTAINER(GetBox()), m_RadioButtons[n]);
    
    if (n < (GetOptions().size()-1))
        gtk_box_reorder_child(GTK_BOX(GetBox()), m_RadioButtons[n], n+1); // +1: Skip title
    
    UpdateScreenLayout();
}

void CLuaRadioButton::DelButton(TSTLVecSize n)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_RadioButtons[n])))
    {
        if (n > 0)
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RadioButtons[n-1]), TRUE);
        else if (n < (GetOptions().size()-1))
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_RadioButtons[n+1]), TRUE);
    }
    
    gtk_widget_destroy(m_RadioButtons[n]);
    m_RadioButtons.erase(m_RadioButtons.begin() + n);
    UpdateScreenLayout();
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
    // 2 signals are created: one by the button that was enabled and one by
    // the button that is now enabled. As we want just one signal catch only the latter.
    if (!radio->m_bInit && gtk_toggle_button_get_active(togglebutton))
        radio->SafeLuaDataChanged();
}
