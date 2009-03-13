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
#include "luacheckbox.h"

// -------------------------------------
// Lua Checkbox Class
// -------------------------------------

CLuaCheckbox::CLuaCheckbox(const char *desc, const TOptions &l,
                           const std::vector<TSTLVecSize> &e) : CBaseLuaWidget(desc), CBaseLuaCheckbox(l), m_bInit(true)
{
    TSTLVecSize size = l.size(), n;
    
    for (n=0; n<size; n++)
    {
        m_Checkboxes.push_back(gtk_check_button_new_with_label(GetTranslation(l[n].c_str())));
        g_signal_connect(G_OBJECT(m_Checkboxes[n]), "toggled", G_CALLBACK(ToggleCB), this);
        gtk_widget_show(m_Checkboxes[n]);
        gtk_container_add(GTK_CONTAINER(GetBox()), m_Checkboxes[n]);
    }
    
    size = e.size();
    for (n=0; n<size; n++)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Checkboxes[e[n]]), TRUE);
        
    m_bInit = false;
}

bool CLuaCheckbox::Enabled(TSTLVecSize n)
{
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_Checkboxes[n])) == TRUE;
}

void CLuaCheckbox::Enable(TSTLVecSize n, bool b)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_Checkboxes[n]), (b) ? TRUE : FALSE);
}

void CLuaCheckbox::AddOption(const std::string &label, TSTLVecSize n)
{
    GtkWidget *button = gtk_check_button_new_with_label(GetTranslation(label.c_str()));
    if ((m_Checkboxes.empty()) || (n >= GetOptions().size()))
        m_Checkboxes.push_back(button);
    else
        m_Checkboxes.insert(m_Checkboxes.begin() + n, button);
    
    g_signal_connect(G_OBJECT(m_Checkboxes[n]), "toggled", G_CALLBACK(ToggleCB), this);
    gtk_widget_show(m_Checkboxes[n]);
    gtk_container_add(GTK_CONTAINER(GetBox()), m_Checkboxes[n]);
    
    if (n < (GetOptions().size()-1))
        gtk_box_reorder_child(GTK_BOX(GetBox()), m_Checkboxes[n], n+1); // +1: Skip title
    
    UpdateScreenLayout();
}

void CLuaCheckbox::DelOption(TSTLVecSize n)
{
    gtk_widget_destroy(m_Checkboxes[n]);
    m_Checkboxes.erase(m_Checkboxes.begin() + n);
    UpdateScreenLayout();
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
    if (!box->m_bInit)
        box->SafeLuaDataChanged();
}
