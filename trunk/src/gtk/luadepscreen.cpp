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
#include "installer.h"
#include "luadepscreen.h"

// -------------------------------------
// GTK Lua Dependency Screen Class
// -------------------------------------

CLuaDepScreen::CLuaDepScreen(GtkWidget *parent, CInstaller *inst, int f) : CBaseLuaDepScreen(f),
                                                                           m_pInstaller(inst), m_bClose(false)
{
    const int windoww = 500, windowh = 0;
    
    m_pDialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_transient_for(GTK_WINDOW(m_pDialog), GTK_WINDOW(parent));
    gtk_window_set_modal(GTK_WINDOW(m_pDialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(m_pDialog), windoww, windowh);
    gtk_window_set_position(GTK_WINDOW(m_pDialog), GTK_WIN_POS_CENTER_ON_PARENT);
    g_signal_connect(G_OBJECT(m_pDialog), "delete_event", G_CALLBACK(DeleteCB), this);
    
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(m_pDialog), vbox);
    
    GtkWidget *title = gtk_label_new(GetTitle());
    gtk_widget_set_size_request(title, windoww-50, -1);
    gtk_label_set_line_wrap(GTK_LABEL(title), TRUE);
    gtk_widget_show(title);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 5);
    
    GtkWidget *list = CreateListBox();
    gtk_widget_show(list);
    gtk_container_add(GTK_CONTAINER(vbox), list);
    
    GtkWidget *bbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(bbox);
    gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 10);
    
    GtkWidget *blabel = gtk_label_new(GetTranslation("Cancel"));
    GtkWidget *button = CreateButton(blabel, GTK_STOCK_CANCEL);
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(CancelCB), this);
    gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, FALSE, 5);
    
    blabel = gtk_label_new(GetTranslation("Refresh"));
    button = CreateButton(blabel, GTK_STOCK_REFRESH);
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(RefreshCB), this);
    gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, FALSE, 5);
    
    blabel = gtk_label_new(GetTranslation("Ignore"));
    button = CreateButton(blabel, GTK_STOCK_STOP);
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(IgnoreCB), this);
    gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, FALSE, 5);
}

CLuaDepScreen::~CLuaDepScreen()
{
    gtk_widget_destroy(m_pDialog);
}

void CLuaDepScreen::CoreUpdateList()
{
    if (!GTK_WIDGET_VISIBLE(GTK_WIDGET(m_pDialog)))
        gtk_widget_show(GTK_WIDGET(m_pDialog));
        
    GtkTreeIter iter;
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(m_pListView)));
    gtk_list_store_clear(store);
    
    for (TDepList::const_iterator it=GetDepList().begin(); it!=GetDepList().end(); it++)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, COLUMN_NAME, GetTranslation(it->name).c_str(),
                           COLUMN_DESC, GetTranslation(it->description).c_str(),
                           COLUMN_PROB, GetTranslation(it->problem).c_str(), -1);
    }
    
    if (GetDepList().empty())
        m_bClose = true;
}

void CLuaDepScreen::CoreRun()
{
    while (!gtk_main_iteration() && !m_bClose)
        ;
}

GtkWidget *CLuaDepScreen::CreateListBox()
{
    const int minh = 150, maxnamew = 100, maxdescw = 250, maxprobw = 250;

    // Scrolling window for treeview
    GtkWidget *sw = gtk_scrolled_window_new(0, 0);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    GtkListStore *store = gtk_list_store_new(COLUMN_N, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    
    m_pListView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_widget_set_size_request(m_pListView, -1, minh);

    g_object_unref(store);
    
    GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pListView));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_NONE);
    
    GtkCellRenderer *cellrenderer = gtk_cell_renderer_text_new();
    g_object_set(GTK_OBJECT(cellrenderer), "wrap-mode", PANGO_WRAP_WORD_CHAR, NULL);
    g_object_set(GTK_OBJECT(cellrenderer), "wrap-width", maxnamew, NULL);
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(GetTranslation("Name"), cellrenderer,
            "text", COLUMN_NAME, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5f);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_pListView), column);

    cellrenderer = gtk_cell_renderer_text_new();
    g_object_set(GTK_OBJECT(cellrenderer), "wrap-mode", PANGO_WRAP_WORD_CHAR, NULL);
    g_object_set(GTK_OBJECT(cellrenderer), "wrap-width", maxdescw, NULL);
    column = gtk_tree_view_column_new_with_attributes(GetTranslation("Description"), cellrenderer, "text", COLUMN_DESC, NULL);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_column_set_alignment(column, 0.5f);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_pListView), column);

    cellrenderer = gtk_cell_renderer_text_new();
    g_object_set(GTK_OBJECT(cellrenderer), "wrap-mode", PANGO_WRAP_WORD_CHAR, NULL);
    g_object_set(GTK_OBJECT(cellrenderer), "wrap-width", maxprobw, NULL);
    column = gtk_tree_view_column_new_with_attributes(GetTranslation("Problem"), cellrenderer, "text", COLUMN_PROB, NULL);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_column_set_alignment(column, 0.5f);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_pListView), column);

    gtk_container_add(GTK_CONTAINER(sw), m_pListView);
    gtk_widget_show_all(sw);
    
    return sw;
}

gboolean CLuaDepScreen::DeleteCB(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    CLuaDepScreen *parent = static_cast<CLuaDepScreen *>(data);
    parent->m_pInstaller->Cancel();
    return TRUE;
}

void CLuaDepScreen::CancelCB(GtkWidget *widget, gpointer data)
{
    CLuaDepScreen *screen = static_cast<CLuaDepScreen *>(data);
    screen->m_pInstaller->Cancel();
}

void CLuaDepScreen::RefreshCB(GtkWidget *widget, gpointer data)
{
    CLuaDepScreen *screen = static_cast<CLuaDepScreen *>(data);
    screen->Refresh();
}

void CLuaDepScreen::IgnoreCB(GtkWidget *widget, gpointer data)
{
    CLuaDepScreen *screen = static_cast<CLuaDepScreen *>(data);
    screen->m_bClose = true;
}
