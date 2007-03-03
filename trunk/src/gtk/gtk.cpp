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

namespace {
CGTKBase *pInterface = NULL;
};

void StartFrontend(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    
    pInterface = new /*CInstaller*/CGTKBase();
    pInterface->Init(argc, argv);
    pInterface->Run();
}

void StopFrontend()
{
    delete pInterface;
    pInterface = NULL;
}

void ReportError(const char *msg)
{
    MessageBox(GTK_MESSAGE_ERROR, msg);
//     fl_message(msg);
}

// -------------------------------------
// Base GTK frontend class
// -------------------------------------

CGTKBase::CGTKBase()
{
    m_pMainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(m_pMainWindow), "destroy", G_CALLBACK(DestroyCB), NULL);
}

char *CGTKBase::GetPassword(const char *str)
{
    const int windoww = 400;
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(GetTranslation("Password dialog"), NULL, GTK_DIALOG_MODAL,
                                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                    NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), windoww, -1);
    
    GtkWidget *label = gtk_label_new(str);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, TRUE, 15);
    gtk_widget_show(label);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    
    GtkWidget *input = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(input), FALSE);
    gtk_box_pack_end(GTK_BOX(hbox), input, TRUE, TRUE, 15);
    gtk_widget_show(input);
    
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
    gtk_widget_show(hbox);
    
    gint dialogret = gtk_dialog_run(GTK_DIALOG(dialog));
    char *ret = (dialogret == GTK_RESPONSE_OK) ? StrDup(gtk_entry_get_text(GTK_ENTRY(input))) : NULL;
    
    gtk_widget_destroy(dialog);

    return ret;
}

void CGTKBase::MsgBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    MessageBox(GTK_MESSAGE_INFO, text);
    
    free(text);
}

void CGTKBase::WarnBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    MessageBox(GTK_MESSAGE_WARNING, text);
    
    free(text);
}

void CGTKBase::Run()
{
    gtk_widget_show(m_pMainWindow);
    char *p = GetPassword("Root access is required to continue\nAbort installation?");
    if (p) MsgBox(p);
    gtk_main ();
}
