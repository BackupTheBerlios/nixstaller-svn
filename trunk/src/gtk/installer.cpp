/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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
#include "luaprogressdialog.h"

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

void CInstaller::SetAboutLabel()
{
    gtk_label_set_markup(GTK_LABEL(m_pAboutLabel),
                         CreateText("<span size=\"small\" color=\"blue\"><u>%s</u></span>", GetTranslation("About")));
}

void CInstaller::InitAboutSection(GtkWidget *parentbox)
{
    // Event box for setting background header
    GdkColor colors;
    gdk_color_parse("white", &colors);
    GtkWidget *eb = gtk_event_box_new();
    gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &colors);
    gtk_box_pack_start(GTK_BOX(parentbox), eb, FALSE, FALSE, 0);

    // VBox so that header can have extra spacing
    GtkWidget *topbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(topbox);
    gtk_container_add(GTK_CONTAINER(eb), topbox);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(topbox), hbox, TRUE, TRUE, 5);

    m_pLogo = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(hbox), m_pLogo, FALSE, FALSE, 5);

    m_pTitle = gtk_label_new(NULL);
    gtk_widget_set_size_request(m_pTitle, 500, -1);
    gtk_label_set_line_wrap(GTK_LABEL(m_pTitle), TRUE);
    gtk_label_set_justify(GTK_LABEL(m_pTitle), GTK_JUSTIFY_CENTER);
    gtk_container_add(GTK_CONTAINER(hbox), m_pTitle);
    
    // Box for getting About link on top
    GtkWidget *aboutbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), aboutbox, FALSE, FALSE, 5);
    
    // Event box so about link can recieve button clicks
    GtkWidget *abouteb = gtk_event_box_new();
    g_signal_connect(G_OBJECT(abouteb), "button-press-event", G_CALLBACK(AboutCB), this);
    g_signal_connect(G_OBJECT(abouteb), "enter_notify_event", G_CALLBACK(AboutEnterCB), NULL);
    g_signal_connect(G_OBJECT(abouteb), "leave_notify_event", G_CALLBACK(AboutLeaveCB), NULL);
    gtk_widget_modify_bg(abouteb, GTK_STATE_NORMAL, &colors);
    gtk_box_pack_start(GTK_BOX(aboutbox), abouteb, FALSE, FALSE, 5);
    
    m_pAboutLabel = gtk_label_new(NULL);
    SetAboutLabel();
    gtk_container_add(GTK_CONTAINER(abouteb), m_pAboutLabel);
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
        gtk_widget_set_sensitive(m_pBackButton, FALSE);
    else if (!m_bPrevButtonLocked)
        gtk_widget_set_sensitive(m_pBackButton, TRUE);
    
    if (LastValidScreen() && !curscreen->HasNextWidgets())
    {
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
    if (m_bBusyActivate)
        return;
        
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
            m_bBusyActivate = true;
            gtk_notebook_set_current_page(GTK_NOTEBOOK(m_pWizard), page);
            screen->Activate();
            screen->SubLast();
            UpdateButtons();
            m_bBusyActivate = false;
            return;
        }
    }
}

void CInstaller::Next(void)
{
    if (m_bBusyActivate)
        return;
        
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
            m_bBusyActivate = true;
            gtk_notebook_set_current_page(GTK_NOTEBOOK(m_pWizard), page);
            screen->Activate();
            UpdateButtons();
            m_bBusyActivate = false;
            return;
        }
    }
    
    // No screens left
    gtk_widget_destroy(GetMainWin());
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
    
    // Logo might be set in lua config, so load it after init
    gtk_image_set_from_file(GTK_IMAGE(m_pLogo), GetLogoFName());
    
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

void CInstaller::CoreUpdateLanguage()
{
    CBaseInstall::CoreUpdateLanguage();
    
    SetAboutLabel();
    
    gtk_label_set(GTK_LABEL(m_pCancelLabel), GetTranslation("Cancel"));
    gtk_label_set(GTK_LABEL(m_pBackLabel), GetTranslation("Back"));
    UpdateButtons(); // Sets Next label

    if (!m_CurTitle.empty())
        SetTitle(m_CurTitle);
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
    const char *msg;
    if (Installing())
        msg = GetTranslation("Install commands are still running\n"
        "If you abort now this may lead to a broken installation\n"
        "Are you sure?");
    else
        msg = GetTranslation("This will abort the installation\nAre you sure?");
    
    return YesNoBox(msg);
}

void CInstaller::CoreUpdateUI()
{
    gtk_main_iteration_do(FALSE);
}

CBaseScreen *CInstaller::CreateScreen(const std::string &title)
{
    return new CInstallScreen(title, this);
}

void CInstaller::CoreAddScreen(CBaseScreen *screen)
{
    CInstallScreen *gtkscreen = dynamic_cast<CInstallScreen *>(screen);
    
    gtkscreen->Init();
    gtk_object_set_user_data(GTK_OBJECT(gtkscreen->GetBox()), gtkscreen);
    gtk_widget_show(gtkscreen->GetBox());
    gtk_notebook_append_page(GTK_NOTEBOOK(m_pWizard), gtkscreen->GetBox(), NULL);
}

CBaseLuaProgressDialog *CInstaller::CoreCreateProgDialog(const std::vector<std::string> &l, int r)
{
    return new CLuaProgressDialog(GetMainWin(), l, r);
}

void CInstaller::LockScreen(bool cancel, bool prev, bool next)
{
    gtk_widget_set_sensitive(m_pCancelButton, !cancel);
    gtk_widget_set_sensitive(m_pBackButton, !prev);
    gtk_widget_set_sensitive(m_pNextButton, !next);
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
    {
        gtk_widget_destroy(parent->GetMainWin());
        // Call this here, because this function may be called during Lua execution
        Quit(EXIT_FAILURE);
    }
}

void CInstaller::BackCB(GtkWidget *widget, gpointer data)
{
    CInstaller *installer = static_cast<CInstaller *>(data);
    
    try
    {
        installer->Back();
    }
    catch(Exceptions::CException &)
    {
        HandleError();
    }
}

void CInstaller::NextCB(GtkWidget *widget, gpointer data)
{
    CInstaller *installer = static_cast<CInstaller *>(data);
    
    try
    {
        installer->Next();
    }
    catch(Exceptions::CException &)
    {
        HandleError();
    }
}

gboolean CInstaller::DeleteCB(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    CInstaller *parent = (CInstaller *)data;
    
    if (parent->AskQuit())
        return FALSE;
    else
        return TRUE;
}
