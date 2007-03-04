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

// -------------------------------------
// Install Frontend Class
// -------------------------------------

void CInstaller::Init(int argc, char **argv)
{
    const int windoww = 600, windowh = 400;
    CBaseInstall::Init(argc, argv);
    
    GtkWidget *mainwin = GetMainWin();
    gtk_window_set_default_size(GTK_WINDOW(mainwin), windoww, windowh);
    
    GtkWidget *abouthbox = gtk_hbox_new(FALSE, 0), *aboutvbox = gtk_vbox_new(FALSE, 0);
    
    GtkWidget *aboutbutton = gtk_button_new()/*gtk_button_new_with_label(GetTranslation("About"))*/;
    g_signal_connect(G_OBJECT(aboutbutton), "clicked", G_CALLBACK(AboutCB), this);
//     gtk_widget_set_size_request(aboutbutton, -1, 20);
    gtk_box_pack_end(GTK_BOX(abouthbox), aboutbutton, FALSE, FALSE, 5);
    gtk_widget_show(aboutbutton);
    
    GtkWidget *butlabel = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(butlabel), CreateText("<span size=\"x-small\">%s</span>", GetTranslation("About")));
    gtk_container_add(GTK_CONTAINER(aboutbutton), butlabel);
    gtk_widget_show(butlabel);

    gtk_box_pack_start(GTK_BOX(aboutvbox), abouthbox, FALSE, FALSE, 0);
    gtk_widget_show(abouthbox);
    
    gtk_container_add(GTK_CONTAINER(mainwin), aboutvbox);
    gtk_widget_show(aboutvbox);
}
