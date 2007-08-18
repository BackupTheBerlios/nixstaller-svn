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
    m_pCancelButton = CreateButton(m_pCancelLabel, GTK_STOCK_CANCEL);
    g_signal_connect(G_OBJECT(m_pCancelButton), "clicked", G_CALLBACK(CancelCB), this);
    gtk_box_pack_start(GTK_BOX(buttonbox), m_pCancelButton, FALSE, FALSE, 5);
    
    gtk_box_pack_start(GTK_BOX(hbox), buttonbox, FALSE, FALSE, 5);
    
    buttonbox = gtk_hbutton_box_new();
    gtk_box_set_spacing(GTK_BOX(buttonbox), 15);
    
    m_pBackLabel = gtk_label_new(GetTranslation("Back"));
    m_pBackButton = CreateButton(m_pBackLabel, GTK_STOCK_GO_BACK);
    g_signal_connect(G_OBJECT(m_pBackButton), "clicked", G_CALLBACK(BackCB), this);
    gtk_box_pack_end(GTK_BOX(buttonbox), m_pBackButton, FALSE, FALSE, 5);

    m_pNextLabel = gtk_label_new(GetTranslation("Next"));
    m_pNextButton = CreateButton(m_pNextLabel, GTK_STOCK_GO_FORWARD);
    g_signal_connect(G_OBJECT(m_pNextButton), "clicked", G_CALLBACK(NextCB), this);
    gtk_box_pack_end(GTK_BOX(buttonbox), m_pNextButton, FALSE, FALSE, 5);

    gtk_box_pack_end(GTK_BOX(hbox), buttonbox, FALSE, FALSE, 5);
    
    gtk_box_pack_start(GTK_BOX(parentbox), hbox, FALSE, FALSE, GetMainSpacing());
}

CInstallScreen *CInstaller::GetScreen(gint index)
{
    GtkWidget *widgetscreen = gtk_notebook_get_nth_page(GTK_NOTEBOOK(m_pWizard), index);
    
    if (!widgetscreen)
        return NULL;
    
    return static_cast<CInstallScreen *>(gtk_object_get_user_data(GTK_OBJECT(widgetscreen)));
}

bool CInstaller::FirstValidScreen()
{
    gint page = gtk_notebook_get_current_page(GTK_NOTEBOOK(m_pWizard));
    
    if (page < 1)
        return true;
    
    CInstallScreen *screen;
    page--;
    while((page >= 0) && (screen = GetScreen(page)))
    {
        if (screen->CanActivate())
            return false;
        page--;
    }
    
    return true;
}

bool CInstaller::LastValidScreen()
{
    gint page = gtk_notebook_get_current_page(GTK_NOTEBOOK(m_pWizard));
    const gint size = gtk_notebook_get_n_pages(GTK_NOTEBOOK(m_pWizard));
    
    if ((page+1) == size)
        return true;
    
    CInstallScreen *screen;
    page++;
    while((page<size) && (screen = GetScreen(page)))
    {
        if (screen->CanActivate())
            return false;
        page++;
    }
    
    return true;
}

void CInstaller::UpdateButtons(void)
{
    CInstallScreen *curscreen = GetScreen(gtk_notebook_get_current_page(GTK_NOTEBOOK(m_pWizard)));
    
    if (FirstValidScreen() && !curscreen->HasPrevWidgets())
        gtk_widget_hide(m_pBackButton);
    else if (!m_bPrevButtonLocked)
        gtk_widget_show(m_pBackButton);
    
    if (LastValidScreen() && !curscreen->HasNextWidgets())
    {
//         gtk_label_set(GTK_LABEL(m_pNextLabel), g_locale_to_utf8(GetTranslation("Finish"), -1, 0, 0, 0));
        gtk_label_set(GTK_LABEL(m_pNextLabel), GetTranslation("Finish"));
        SetButtonStock(m_pNextButton, GTK_STOCK_QUIT);
    }
    else
    {
        gtk_label_set(GTK_LABEL(m_pNextLabel), GetTranslation("Next"));
        SetButtonStock(m_pNextButton, GTK_STOCK_GO_FORWARD);
    }
}

void CInstaller::Back(void)
{
    gint page = gtk_notebook_get_current_page(GTK_NOTEBOOK(m_pWizard));
    CInstallScreen *screen = GetScreen(page);
    
    if (screen->SubBack())
    {
        UpdateButtons();
        return;
    }
    
    if (!screen->Back())
        return;
    
    while (page && screen)
    {
        page--;
        screen = GetScreen(page);
        
        if (screen && screen->CanActivate())
        {
            gtk_notebook_set_current_page(GTK_NOTEBOOK(m_pWizard), page);
            screen->Activate();
            UpdateButtons();
            return;
        }
    }
}

void CInstaller::Next(void)
{
    gint page = gtk_notebook_get_current_page(GTK_NOTEBOOK(m_pWizard));
    CInstallScreen *screen = GetScreen(page);
    
    if (screen->SubNext())
    {
        UpdateButtons();
        return;
    }
    
    if (!screen->Next())
        return;
    
    while (screen)
    {
        page++;
        screen = GetScreen(page);
        
        if (screen && screen->CanActivate())
        {
            gtk_notebook_set_current_page(GTK_NOTEBOOK(m_pWizard), page);
            screen->Activate();
            UpdateButtons();
            return;
        }
    }
    
    // No screens left
    gtk_main_quit();
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
    
    // Activate first screen
    gint page = 0;
    CInstallScreen *screen;
    while ((screen = GetScreen(page)))
    {
        if (screen->CanActivate())
        {
            gtk_notebook_set_current_page(GTK_NOTEBOOK(m_pWizard), page);
            screen->Activate();
            UpdateButtons();
            break;
        }
        page++;
    }
}

void CInstaller::UpdateLanguage()
{
    CBaseInstall::UpdateLanguage();
    
    gtk_label_set(GTK_LABEL(m_pCancelLabel), GetTranslation("Cancel"));
    gtk_label_set(GTK_LABEL(m_pBackLabel), GetTranslation("Back"));
    gtk_label_set(GTK_LABEL(m_pNextLabel), GetTranslation("Next"));

    if (!m_CurTitle.empty())
        SetTitle(m_CurTitle);
    
    gint page = 0;
    CInstallScreen *screen;
    while ((screen = GetScreen(page)))
    {
        screen->UpdateLanguage();
        page++;
    }
}

void CInstaller::SetTitle(const std::string &t)
{
    m_CurTitle = t;
    gchar *markup = g_markup_printf_escaped("<span size=\"x-large\">%s</span>", GetTranslation(t.c_str()));
    gtk_label_set_markup(GTK_LABEL(m_pTitle), markup);
    g_free(markup);
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
    
    screen->Init();

    gtk_object_set_user_data(GTK_OBJECT(screen->GetBox()), screen);
    gtk_widget_show(screen->GetBox());
    gtk_notebook_append_page(GTK_NOTEBOOK(m_pWizard), screen->GetBox(), NULL);
}

void CInstaller::InstallThink()
{
    gtk_main_iteration_do(FALSE);
}

void CInstaller::LockScreen(bool cancel, bool prev, bool next)
{
    SetWidgetVisible(m_pCancelButton, !cancel);
    SetWidgetVisible(m_pBackButton, !prev);
    SetWidgetVisible(m_pNextButton, !next);
    m_bPrevButtonLocked = prev;
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
