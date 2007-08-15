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

namespace {


void SetHandCursor(GtkWidget *widget, bool on)
{
    GdkDisplay *display = gtk_widget_get_display(widget);

    GdkCursor *cursor = NULL;
    if (on)
        cursor = gdk_cursor_new_for_display(display, GDK_HAND2);

    gdk_window_set_cursor(widget->window, cursor);
    gdk_display_flush(display);

    if (cursor)
        gdk_cursor_unref(cursor);
}


}

// -------------------------------------
// Install Frontend Class
// -------------------------------------

void CInstaller::InitAboutSection(GtkWidget *parentbox)
{
    GdkColor colors;
    colors.red = colors.blue = colors.green = 65535;
    
    GtkWidget *eb = gtk_event_box_new();
    gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &colors);
    gtk_box_pack_start(GTK_BOX(parentbox), eb, FALSE, FALSE, 0);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(eb), hbox);

    GtkWidget *image = gtk_image_new_from_file("/home/rick/out_of_the_box_nicu_bucu_01.png");
    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 5);

    GtkWidget *titlebox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), titlebox, TRUE, TRUE, 0);
    
    m_pTitle = gtk_label_new(NULL);
    gtk_widget_set_size_request(m_pTitle, 500, -1);
    gtk_label_set_line_wrap(GTK_LABEL(m_pTitle), TRUE);
    gtk_label_set_justify(GTK_LABEL(m_pTitle), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(titlebox), m_pTitle, TRUE, TRUE, 15);
    
    GtkWidget *aboutbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), aboutbox, FALSE, FALSE, 5);
    
    GtkWidget *abouteb = gtk_event_box_new();
    g_signal_connect(G_OBJECT(abouteb), "button-press-event", G_CALLBACK(AboutCB), this);
    g_signal_connect(G_OBJECT(abouteb), "enter_notify_event", G_CALLBACK(AboutEnterCB), NULL);
    g_signal_connect(G_OBJECT(abouteb), "leave_notify_event", G_CALLBACK(AboutLeaveCB), NULL);
    gtk_widget_modify_bg(abouteb, GTK_STATE_NORMAL, &colors);
    gtk_box_pack_start(GTK_BOX(aboutbox), abouteb, FALSE, FALSE, 5);
    
    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label),
                         CreateText("<span size=\"small\" color=\"blue\"><u>%s</u></span>", GetTranslation("About")));
    gtk_container_add(GTK_CONTAINER(abouteb), label);
}

void CInstaller::InitScreenSection(GtkWidget *parentbox)
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(parentbox), hbox, TRUE, TRUE, 0);
    
    m_pWizard = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(m_pWizard), FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), m_pWizard, TRUE, TRUE, 0);
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
    
    gtk_box_pack_start(GTK_BOX(parentbox), hbox, FALSE, FALSE, GetMainSpacing());
}

void CInstaller::Back(void)
{
    gtk_notebook_prev_page(GTK_NOTEBOOK(m_pWizard));
    
    GtkWidget *widgetscreen = gtk_notebook_get_nth_page(GTK_NOTEBOOK(m_pWizard),
                                                        gtk_notebook_get_current_page(GTK_NOTEBOOK(m_pWizard)));
    
    assert(widgetscreen); // UNDONE?
    
    CInstallScreen *screen = static_cast<CInstallScreen *>(gtk_object_get_user_data(GTK_OBJECT(widgetscreen)));
    screen->Activate();
}

void CInstaller::Next(void)
{
    gtk_notebook_next_page(GTK_NOTEBOOK(m_pWizard));
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

void CInstaller::SetTitle(const std::string &t)
{
    gtk_label_set_markup(GTK_LABEL(m_pTitle), CreateText("<span size=\"x-large\">%s</span>", t.c_str()));
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
    return new CInstallScreen(title, this);
}

void CInstaller::AddScreen(int luaindex)
{
    CInstallScreen *screen = dynamic_cast<CInstallScreen *>(NLua::CheckClassData<CBaseScreen>("screen", luaindex));
    
    while (screen)
    {
        gtk_object_set_user_data(GTK_OBJECT(screen->GetBox()), screen);
        gtk_widget_show(screen->GetBox());
        gtk_notebook_append_page(GTK_NOTEBOOK(m_pWizard), screen->GetBox(), NULL);
        screen = screen->GetNextSubScreen();
    }
}

gboolean CInstaller::AboutEnterCB(GtkWidget *widget, GdkEventCrossing *crossing, gpointer data)
{
    SetHandCursor(widget, true);
    return FALSE;
}

gboolean CInstaller::AboutLeaveCB(GtkWidget *widget, GdkEventCrossing *crossing, gpointer data)
{
    SetHandCursor(widget, false);
    return FALSE;
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
