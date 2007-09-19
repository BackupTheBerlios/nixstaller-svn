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
#include "luadirselector.h"

// -------------------------------------
// Lua directory selector Class
// -------------------------------------

CLuaDirSelector::CLuaDirSelector(const char *desc, const char *val) : CBaseLuaWidget(desc)
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 10);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(GetBox()), hbox);
    
    m_pDirInput = gtk_entry_new();
    SetDir(val);
    g_signal_connect(G_OBJECT(m_pDirInput), "changed", G_CALLBACK(InputChangedCB), this);
    gtk_widget_show(m_pDirInput);
    gtk_container_add(GTK_CONTAINER(hbox), m_pDirInput);
    
    GtkWidget *button = CreateButton(m_pDirButtonLabel = gtk_label_new(GetTranslation("Browse")), GTK_STOCK_OPEN);
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(BrowseCB), m_pDirInput);
    gtk_widget_show(button);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
}

const char *CLuaDirSelector::GetDir(void)
{
    return gtk_entry_get_text(GTK_ENTRY(m_pDirInput));
}

void CLuaDirSelector::SetDir(const char *dir)
{
    gtk_entry_set_text(GTK_ENTRY(m_pDirInput), dir);
}

void CLuaDirSelector::CoreUpdateLanguage()
{
    gtk_label_set(GTK_LABEL(m_pDirButtonLabel), GetTranslation("Browse"));
}

void CLuaDirSelector::CoreActivateWidget()
{
    gtk_widget_grab_focus(m_pDirInput);
}

void CLuaDirSelector::InputChangedCB(GtkEditable *widget, gpointer data)
{
    CLuaDirSelector *dirselector = static_cast<CLuaDirSelector *>(data);
    dirselector->SafeLuaDataChanged();
}

void CLuaDirSelector::BrowseCB(GtkWidget *widget, gpointer data)
{
    GtkWidget *entry = static_cast<GtkWidget *>(data);
    GtkWidget *dialog = CreateDirChooser(CreateText(GetTranslation("Select a directory")));
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), gtk_entry_get_text(GTK_ENTRY(entry)));
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(entry), filename);
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}
