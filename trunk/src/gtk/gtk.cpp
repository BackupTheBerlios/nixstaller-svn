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

namespace {
CGTKBase *pInterface = NULL;
};

#ifndef RELEASE
void debugline(const char *t, ...)
{
    char *txt;
    va_list v;
    
    va_start(v, t);
    vasprintf(&txt, t, v);
    va_end(v);
    
    printf("DEBUG: %s", txt);
    
    free(txt);
}
#endif

void StartFrontend(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    
    pInterface = new CInstaller();
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
}

// -------------------------------------
// Base GTK frontend class
// -------------------------------------

CGTKBase::CGTKBase()
{
    g_set_application_name("Nixstaller");
    m_pMainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(m_pMainWindow), "destroy", G_CALLBACK(DestroyCB), NULL);
    CreateAbout();
}

void CGTKBase::CreateAbout()
{
    const int windoww = 350, windowh = 350;
    
    m_pAboutDialog = gtk_dialog_new_with_buttons(GetTranslation("About"), GTK_WINDOW(m_pMainWindow),
                                                 GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                                 GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
    gtk_window_set_default_size(GTK_WINDOW(m_pAboutDialog), windoww, windowh);
    
    GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
    GtkTextIter iter;
    char chbuf[1024];
    FILE *aboutfile = fopen(GetAboutFName(), "r");
    
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
    
    while (aboutfile && fgets(chbuf, sizeof(chbuf), aboutfile))
    {
        gtk_text_buffer_insert(buffer, &iter, chbuf, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", 1);
    }
    
    if (aboutfile)
        fclose(aboutfile);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkWidget *textview = gtk_text_view_new_with_buffer(GTK_TEXT_BUFFER(buffer));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled), textview);
    gtk_widget_show(textview);
    
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_pAboutDialog)->vbox), scrolled);
    gtk_widget_show(scrolled);
}

void CGTKBase::ShowAbout()
{
    gtk_dialog_run(GTK_DIALOG(m_pAboutDialog));
    gtk_widget_hide(m_pAboutDialog);
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

bool CGTKBase::YesNoBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, text);
    bool yes = (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES);
    gtk_widget_destroy(dialog);
    
    free(text);
    
    return yes;
}

int CGTKBase::ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...)
{
    char *text;
    va_list v;
    
    va_start(v, button3);
    vasprintf(&text, str, v);
    va_end(v);
    
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, text);
    gtk_dialog_add_buttons(GTK_DIALOG(dialog), button1, 0, button2, 1, button3, 2, NULL);
    int ret = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    free(text);
    
    return ret;
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
    CreateAbout(); // Create after everything is initialized: only then GetAboutFName() returns a valid filename
    gtk_widget_show(m_pMainWindow);
    gtk_main();
}
