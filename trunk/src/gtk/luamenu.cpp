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

#include "main/main.h"
#include "gtk.h"
#include "luamenu.h"

// -------------------------------------
// Lua Menu Class
// -------------------------------------

CLuaMenu::CLuaMenu(const char *desc, const TOptions &l,
                   TSTLVecSize e) : CBaseLuaWidget(desc), CBaseLuaMenu(l), m_bInitSel(true)
{
    gtk_container_add(GTK_CONTAINER(GetBox()), CreateMenu());
    TSTLVecSize n = 0;
    
    for (TOptions::const_iterator it=l.begin(); it!=l.end(); it++, n++)
    {
        GtkTreeIter iter;
        GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(m_pMenu)));
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, COLUMN_TITLE, GetTranslation(it->c_str()), COLUMN_VAR,
                           it->c_str(), -1);
    
        if (n == e)
        {
            GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pMenu));
            gtk_tree_selection_select_iter(selection, &iter);
            m_bInitSel = false;
        }
    }
}

std::string CLuaMenu::Selection()
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pMenu));
    GtkTreeIter iter;
    GtkTreeModel *model;
    std::string ret;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gchar *var;
        gtk_tree_model_get(model, &iter, COLUMN_VAR, &var, -1);
        ret = var;
        g_free(var);
    }
    
    return ret;
}

void CLuaMenu::Select(TSTLVecSize n)
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pMenu));
    GtkTreeIter iter;
    
    if (SearchItem(n, iter))
    {
        gtk_tree_selection_select_iter(selection, &iter);
        m_bInitSel = false;
    }
}

void CLuaMenu::AddOption(const std::string &label)
{
    GtkTreeIter iter;
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(m_pMenu)));
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, COLUMN_TITLE, GetTranslation(label.c_str()), COLUMN_VAR,
                       label.c_str(), -1);
    if (m_bInitSel)
    {
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pMenu));
        gtk_tree_selection_select_iter(selection, &iter);
        m_bInitSel = false;
    }
}

void CLuaMenu::DelOption(TSTLVecSize n)
{
    GtkTreeIter iter;
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(m_pMenu)));
    
    if (SearchItem(n, iter))
        gtk_list_store_remove(store, &iter);
    else
        assert(false); // Should not happen
}

void CLuaMenu::CoreUpdateLanguage()
{
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(m_pMenu)));
    GtkTreeIter it;

    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &it))
    {
        do
        {
            gchar *var;
            gtk_tree_model_get(GTK_TREE_MODEL(store), &it, COLUMN_VAR, &var, -1);
            gtk_list_store_set(store, &it, COLUMN_TITLE, GetTranslation(var), -1);
            g_free(var);
        }
        while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &it));
    }
}

void CLuaMenu::CoreActivateWidget()
{
    gtk_widget_grab_focus(m_pMenu);
}

GtkWidget *CLuaMenu::CreateMenu()
{
    // Scrolling window for treeview
    GtkWidget *sw = gtk_scrolled_window_new(0, 0);
    gtk_widget_set_size_request(sw, -1, 150);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    GtkListStore *store = gtk_list_store_new(COLUMN_N, G_TYPE_STRING, G_TYPE_STRING);
    
    m_pMenu = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_pMenu), FALSE);
    
    g_object_unref(store);
    
    GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pMenu));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(select), "changed", G_CALLBACK(SelectionCB), this);
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("", renderer, "text",
                                                                         COLUMN_TITLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_pMenu), column);

    gtk_container_add(GTK_CONTAINER(sw), m_pMenu);
    gtk_widget_show_all(sw);
    
    return sw;
}

bool CLuaMenu::SearchItem(TSTLVecSize n, GtkTreeIter &iter)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_pMenu));
    
    if (!gtk_tree_model_get_iter_first(model, &iter))
        return false;
    
    bool found = false;
    do
    {
        gchar *var;
        gtk_tree_model_get(model, &iter, COLUMN_VAR, &var, -1);
        if (var == GetOptions()[n])
            found = true;
        g_free(var);
    }
    while (!found && gtk_tree_model_iter_next(model, &iter));
    
    return found;
}

void CLuaMenu::SelectionCB(GtkTreeSelection *selection, gpointer data)
{
    CLuaMenu *menu = static_cast<CLuaMenu *>(data);
    if (!menu->m_bInitSel)
        menu->SafeLuaDataChanged();
}
