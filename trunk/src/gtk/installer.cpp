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

#include "main/lua/luaclass.h"
#include "gtk.h"
#include "installer.h"
#include "installscreen.h"

// -------------------------------------
// Install Frontend Class
// -------------------------------------

void CInstaller::InitAboutSection(GtkWidget *parentbox)
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    
    GtkWidget *button = gtk_button_new();
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(AboutCB), this);
    gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 5);
    
    GtkWidget *butlabel = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(butlabel), CreateText("<span size=\"x-small\">%s</span>", GetTranslation("About")));
    gtk_container_add(GTK_CONTAINER(button), butlabel);
    
    gtk_box_pack_start(GTK_BOX(parentbox), hbox, FALSE, FALSE, 0);
}

void CInstaller::InitScreenSection(GtkWidget *parentbox)
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 10);
    
    m_pWizard = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(m_pWizard), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(m_pWizard), FALSE);
    
//     CBaseScreen *p = new CBaseScreen(this);
//     GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
//     p->CreateScreen(vbox);
//     gtk_notebook_append_page(GTK_NOTEBOOK(m_pWizard), vbox, NULL);
    
    gtk_container_add(GTK_CONTAINER(frame), m_pWizard);
    
    gtk_box_pack_start(GTK_BOX(parentbox), hbox, TRUE, TRUE, 10);
}

void CInstaller::InitButtonSection(GtkWidget *parentbox)
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    
    GtkWidget *buttonbox = gtk_hbutton_box_new();
    
    m_pCancelLabel = gtk_label_new(GetTranslation("Cancel"));
    GtkWidget *button = CreateButton(m_pCancelLabel, GTK_STOCK_CANCEL);
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(CancelCB), this);
    gtk_box_pack_start(GTK_BOX(buttonbox), button, FALSE, FALSE, 5);
    
    gtk_box_pack_start(GTK_BOX(hbox), buttonbox, FALSE, FALSE, 5);
    
    buttonbox = gtk_hbutton_box_new();
    gtk_box_set_spacing(GTK_BOX(buttonbox), 15);
    
    m_pBackLabel = gtk_label_new(GetTranslation("Back"));
    button = CreateButton(m_pBackLabel, GTK_STOCK_GO_BACK);
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(BackCB), this);
    gtk_box_pack_end(GTK_BOX(buttonbox), button, FALSE, FALSE, 5);

    m_pNextLabel = gtk_label_new(GetTranslation("Next"));
    button = CreateButton(m_pNextLabel, GTK_STOCK_GO_FORWARD);
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(NextCB), this);
    gtk_box_pack_end(GTK_BOX(buttonbox), button, FALSE, FALSE, 5);

    gtk_box_pack_end(GTK_BOX(hbox), buttonbox, FALSE, FALSE, 5);
    
    gtk_box_pack_start(GTK_BOX(parentbox), hbox, FALSE, FALSE, 5);
}

void CInstaller::Init(int argc, char **argv)
{
    const int windoww = 600, windowh = 400;
    
    GtkWidget *mainwin = GetMainWin();
    g_signal_connect(G_OBJECT(mainwin), "delete_event", G_CALLBACK(DeleteCB), this);
    gtk_window_set_default_size(GTK_WINDOW(mainwin), windoww, windowh);
    
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

    InitAboutSection(vbox);
    InitScreenSection(vbox);
    InitButtonSection(vbox);

    gtk_widget_show_all(vbox);
    gtk_container_add(GTK_CONTAINER(mainwin), vbox);

    CBaseInstall::Init(argc, argv);
}

bool CInstaller::AskQuit()
{
    char *msg;
    if (Installing())
        msg = GetTranslation("Install commands are still running\n"
        "If you abort now this may lead to a broken installation\n"
        "Are you sure?");
    else
        msg = GetTranslation("This will abort the installation\nAre you sure?");
    
    return YesNoBox(msg);
}

CBaseScreen *CInstaller::CreateScreen(const std::string &title)
{
    return new CInstallScreen(title);
}

void CInstaller::AddScreen(int luaindex)
{
    CInstallScreen *screen = dynamic_cast<CInstallScreen *>(NLua::CheckClassData<CBaseScreen>("screen", luaindex));
    
    while (screen)
    {
        gtk_widget_show(screen->GetBox());
        gtk_notebook_append_page(GTK_NOTEBOOK(m_pWizard), screen->GetBox(), NULL);
        screen = screen->GetNextSubScreen();
    }
}

void CInstaller::CancelCB(GtkWidget *widget, gpointer data)
{
    CInstaller *parent = (CInstaller *)data;
    
    if (parent->AskQuit())
        gtk_widget_destroy(parent->GetMainWin());
}

gboolean CInstaller::DeleteCB(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    CInstaller *parent = (CInstaller *)data;
    
    if (parent->AskQuit())
        return FALSE;
    else
        return TRUE;
}
