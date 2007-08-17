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

void MessageBox(GtkMessageType type, const char *msg)
{
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, type, GTK_BUTTONS_OK, msg);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

GtkWidget *CreateButton(GtkWidget *label, const gchar *image, bool fromstock)
{
    GtkWidget *button = gtk_button_new();
    
    if (image)
    {
        GtkWidget *alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
        
        GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
        
        GtkWidget *img = (fromstock) ? gtk_image_new_from_stock(image, GTK_ICON_SIZE_BUTTON) : gtk_image_new_from_file(image);
        gtk_box_pack_start(GTK_BOX(hbox), img, FALSE, FALSE, 0);
        gtk_widget_show(img);
        
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        gtk_widget_show(label);

        gtk_container_add(GTK_CONTAINER(alignment), hbox);
        gtk_widget_show(hbox);
    
        gtk_container_add(GTK_CONTAINER(button), alignment);
        gtk_widget_show(alignment);
    }
    else
    {
        gtk_container_add(GTK_CONTAINER(button), label);
        gtk_widget_show(label);
    }
    
    gtk_widget_show(button);
    
    return button;
}

// Works only for buttons created with CreateButton()
void SetButtonStock(GtkWidget *button, const gchar *image)
{
    GtkWidget *alignment = gtk_bin_get_child(GTK_BIN(button));
    GtkWidget *hbox = gtk_bin_get_child(GTK_BIN(alignment));
    GList *list = gtk_container_get_children(GTK_CONTAINER(hbox));
    GtkWidget *img = GTK_WIDGET(list->data);
    g_list_free(list);
    gtk_image_set_from_stock(GTK_IMAGE(img), image, GTK_ICON_SIZE_BUTTON);
}
