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

#include "main/frontend/utils.h"
#include "gtk.h"

namespace {

bool g_bUpdateFileName = false;

// -------------------------------------
// Callbacks for CreateDirChooser()
// -------------------------------------

void UpdateDirSelCB(GtkFileChooser *widget, gpointer data)
{
    if (g_bUpdateFileName)
    {
        g_bUpdateFileName = false;
        debugline("set_filename: %s\n", static_cast<char *>(data));
        gtk_file_chooser_set_filename(widget, static_cast<char *>(data));
    }
    // Only do it once (DISABLED for now, as this crashes on older GTK versions)
    g_signal_handlers_disconnect_by_func(G_OBJECT(widget),
                                        (gpointer)(UpdateDirSelCB), data);
}

void CreateRootDirCB(GtkWidget *widget, gpointer data)
{
    GtkWidget *filedialog = static_cast<GtkWidget *>(data);
    char *curdir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(filedialog));
    GtkWidget *dialog = gtk_dialog_new_with_buttons(GetTranslation("Create new directory"), NULL,
            GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
    
    gtk_box_set_spacing(GTK_BOX(GTK_DIALOG(dialog)->vbox), 10);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 120);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    
    GtkWidget *hbox = gtk_hbox_new(FALSE, 10);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
    
    GtkWidget *label = gtk_label_new(GetTranslation("Directory name"));
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    GtkWidget *direntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(direntry), (curdir) ? curdir : "/");
    gtk_entry_set_activates_default(GTK_ENTRY(direntry), TRUE);
    gtk_widget_show(direntry);
    gtk_container_add(GTK_CONTAINER(hbox), direntry);
    
    hbox = gtk_hbox_new(FALSE, 10);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);

    label = gtk_label_new(GetTranslation("Root password"));
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    GtkWidget *passentry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(passentry), FALSE);
    gtk_entry_set_activates_default(GTK_ENTRY(passentry), TRUE);
    gtk_widget_show(passentry);
    gtk_container_add(GTK_CONTAINER(hbox), passentry);

    LIBSU::CLibSU suhandler;
    bool cancelled = true;
    while (true)
    {
        if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
            break;
        
        if (!suhandler.TestSU(gtk_entry_get_text(GTK_ENTRY(passentry))))
        {
            if (suhandler.GetError() == LIBSU::CLibSU::SU_ERROR_INCORRECTPASS)
            {
                MessageBox(GTK_MESSAGE_WARNING, GetTranslation("Incorrect password given, please retype."));
                continue;
            }
            else
            {
                MessageBox(GTK_MESSAGE_WARNING, GetTranslation("Could not use su or sudo to gain root access."));
                break;
            }
        }
        
        cancelled = false;
        break;
    }
    
    if (!cancelled)
    {
        const char *newdir = gtk_entry_get_text(GTK_ENTRY(direntry));
        
        try
        {
            MKDirRecRoot(newdir, suhandler, gtk_entry_get_text(GTK_ENTRY(passentry)));
            
            // This is rather hacky... First the current viewing directory is changed to the new directory,
            // this will trigger an internal update and emits a signal. After this signal is launched the
            // current selected directory is changed to the new directory. We have to force an update, because
            // otherwise the new directory cannot be selected.
            g_bUpdateFileName = true;
            g_signal_connect_after(G_OBJECT(filedialog), "current-folder-changed",
                                   G_CALLBACK(UpdateDirSelCB), CreateText(newdir));
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(filedialog), newdir);
            debugline("current_folder: %s\n", newdir);
        }
        catch(Exceptions::CExIO &e)
        {
            MessageBox(GTK_MESSAGE_WARNING, e.what());
        }
    }
    
    gtk_widget_destroy(dialog);
    
    if (curdir)
        g_free(curdir);
}


}

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
    CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(hbox)), g_list_free);
    GtkWidget *img = GTK_WIDGET(list->data);
    gtk_image_set_from_stock(GTK_IMAGE(img), image, GTK_ICON_SIZE_BUTTON);
}

void SetWidgetVisible(GtkWidget *w, bool v)
{
    if (v)
        gtk_widget_show(w);
    else
        gtk_widget_hide(w);
}

GtkWidget *CreateDirChooser(const char *title)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(title, NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
                                                    GTK_RESPONSE_ACCEPT, NULL);
    
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    
    GtkWidget *rootbutton = CreateButton(gtk_label_new(GetTranslation("Create directory as root")));
    g_signal_connect(G_OBJECT(rootbutton), "clicked", G_CALLBACK(CreateRootDirCB), dialog);
    gtk_widget_show(rootbutton);
    gtk_box_pack_end(GTK_BOX(hbox), rootbutton, FALSE, FALSE, 0);
    
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), hbox);
    
    return dialog;
}

bool ContainerEmpty(GtkContainer *c)
{
    CPointerWrapper<GList> list(gtk_container_get_children(c), g_list_free);
    if (!list)
        return true;
    
    return false;
}

int ContainerSize(GtkContainer *c)
{
    int ret = 0;
    
    CPointerWrapper<GList> list(gtk_container_get_children(c), g_list_free);
    GList *entry = list;
    for (; entry; entry=g_list_next(entry), ret++)
        ;
    
    return ret;
}

int TextWidth(const char *str)
{
    // Uses a dummy label widget to retrieve the requested width from str
    static GtkWidget *lab = NULL;
    
    if (!lab)
        lab = gtk_label_new(str);
    else
        gtk_label_set(GTK_LABEL(lab), str);
    
    GtkRequisition req;
    gtk_widget_size_request(lab, &req);
    
    return req.width;
}
