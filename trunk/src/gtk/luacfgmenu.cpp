/*
    Copyright (C) 2007 Rick Helmus (rhelmus_AT_gmail.com)

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
#include "luacfgmenu.h"

// -------------------------------------
// Lua Config Menu Class
// -------------------------------------

CLuaCFGMenu::CLuaCFGMenu(const char *desc) : CBaseLuaWidget(desc), m_bInitSelection(true)
{
    GtkWidget *vbox = gtk_vbox_new(FALSE, 10);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(GetBox()), vbox);
    
    GtkWidget *varbox = CreateVarListBox();
    gtk_container_add(GTK_CONTAINER(vbox), varbox);
    
    m_pInputField = CreateInputField();
    gtk_box_pack_start(GTK_BOX(vbox), m_pInputField, TRUE, TRUE, 10);
    
    m_pComboBox = CreateComboBox();
    gtk_box_pack_start(GTK_BOX(vbox), m_pComboBox, TRUE, TRUE, 10);
    
    m_pDirInputBox = CreateDirSelector();
    gtk_box_pack_start(GTK_BOX(vbox), m_pDirInputBox, TRUE, TRUE, 10);
}

GtkWidget *CLuaCFGMenu::CreateVarListBox()
{
    // Scrolling window for treeview
    GtkWidget *sw = gtk_scrolled_window_new(0, 0);
    gtk_widget_set_size_request(sw, -1, 175);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    GtkListStore *store = gtk_list_store_new(COLUMN_N, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    
    m_pVarListView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    
    g_object_unref(store);
    
    GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pVarListView));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(select), "changed", G_CALLBACK(SelectionCB), this);
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Variable", renderer, "text", COLUMN_TITLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_pVarListView), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(GTK_OBJECT(renderer), "wrap-width", MaxWidgetWidth()-100);
    column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", COLUMN_DESC, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_pVarListView), column);

    gtk_container_add(GTK_CONTAINER(sw), m_pVarListView);
    gtk_widget_show_all(sw);
    
    return sw;
}

GtkWidget *CLuaCFGMenu::CreateInputField()
{
    GtkWidget *ret = gtk_entry_new();
    g_signal_connect(G_OBJECT(ret), "changed", G_CALLBACK(InputChangedCB), this);
    return ret;
}

GtkWidget *CLuaCFGMenu::CreateComboBox()
{
    GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
    
    GtkWidget *combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(ComboBoxChangedCB), this);
    
    g_object_unref (store);

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "text", 0, NULL);

    return combo;
}

GtkWidget *CLuaCFGMenu::CreateDirSelector()
{
    GtkWidget *box = gtk_hbox_new(FALSE, 10);
    
    m_pDirInput = gtk_entry_new();
    gtk_widget_show(m_pDirInput);
    g_signal_connect(G_OBJECT(m_pDirInput), "changed", G_CALLBACK(InputChangedCB), this);
    gtk_container_add(GTK_CONTAINER(box), m_pDirInput);
    
    GtkWidget *button = CreateButton(m_pDirButtonLabel = gtk_label_new(GetTranslation("Browse")), GTK_STOCK_OPEN);
    gtk_widget_show(button);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
    
    return box;
}

void CLuaCFGMenu::UpdateSelection(const gchar *var)
{
    gtk_widget_hide(m_pInputField);
    gtk_widget_hide(m_pComboBox);
    gtk_widget_hide(m_pDirInputBox);
    
    const SEntry *entry = GetVariables()[var];
    
    switch (entry->type)
    {
        case TYPE_STRING:
            gtk_entry_set_text(GTK_ENTRY(m_pInputField), entry->val.c_str());
            gtk_widget_show(m_pInputField);
            break;
        case TYPE_DIR:
            gtk_entry_set_text(GTK_ENTRY(m_pDirInput), entry->val.c_str());
            gtk_widget_show(m_pDirInputBox);
            break;
        case TYPE_BOOL:
        case TYPE_LIST:
            SetComboBox(var);
            gtk_widget_show(m_pComboBox);
            break;
    }
}

void CLuaCFGMenu::SetComboBox(const std::string &var)
{
    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(m_pComboBox)));
    
    gtk_list_store_clear(store);
    
    SEntry *entry = GetVariables()[var];
    GtkTreeIter iter;
    for (TOptionsType::iterator it=entry->options.begin(); it!=entry->options.end(); it++)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, GetTranslation(it->c_str()), -1);
        
        if (*it == entry->val)
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(m_pComboBox), &iter);
    }
}

void CLuaCFGMenu::CoreAddVar(const char *name)
{
    GtkTreeIter iter;
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(m_pVarListView)));
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, COLUMN_TITLE, GetTranslation(name),
                       COLUMN_DESC, GetTranslation(GetVariables()[name]->desc.c_str()),
                       COLUMN_VAR, name, -1);
    
    if (m_bInitSelection)
    {
        m_bInitSelection = false;
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pVarListView));
        gtk_tree_selection_select_iter(selection, &iter);
    }
}

void CLuaCFGMenu::CoreUpdateLanguage()
{
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(m_pVarListView)));
    GtkTreeIter it;
    
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &it))
    {
        do
        {
            gchar *var;
            gtk_tree_model_get(GTK_TREE_MODEL(store), &it, COLUMN_VAR, &var, -1);
            gtk_list_store_set(store, &it, COLUMN_TITLE, GetTranslation(var),
                               COLUMN_DESC, GetTranslation(GetVariables()[var]->desc.c_str()), -1);
            g_free(var);
        }
        while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &it));
    }
    
    gtk_label_set(GTK_LABEL(m_pDirButtonLabel), GetTranslation("Browse"));
    
    std::string sel = CurSelection();
    if (!sel.empty() && ((GetVariables()[sel]->type == TYPE_BOOL) || (GetVariables()[sel]->type == TYPE_LIST)))
        SetComboBox(sel);
}

void CLuaCFGMenu::CoreActivateWidget()
{
    gtk_widget_grab_focus(m_pVarListView);
}

std::string CLuaCFGMenu::CurSelection()
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pVarListView));
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

void CLuaCFGMenu::SelectionCB(GtkTreeSelection *selection, gpointer data)
{
    CLuaCFGMenu *menu = static_cast<CLuaCFGMenu *>(data);
    std::string var = menu->CurSelection();

    if (!var.empty())
        menu->UpdateSelection(var.c_str());
}

void CLuaCFGMenu::InputChangedCB(GtkEditable *widget, gpointer data)
{
    CLuaCFGMenu *menu = static_cast<CLuaCFGMenu *>(data);
    std::string selection = menu->CurSelection();

    if (!selection.empty())
    {
        menu->GetVariables()[selection]->val = gtk_entry_get_text(GTK_ENTRY(widget));
        menu->LuaDataChanged();
    }
}

void CLuaCFGMenu::ComboBoxChangedCB(GtkComboBox *widget, gpointer data)
{
    CLuaCFGMenu *menu = static_cast<CLuaCFGMenu *>(data);
    std::string selection = menu->CurSelection();

    if (!selection.empty())
    {
        GtkTreeIter iter;
        if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(menu->m_pComboBox), &iter))
        {
            gchar *text;
            gtk_tree_model_get(gtk_combo_box_get_model(GTK_COMBO_BOX(menu->m_pComboBox)), &iter,
                               0, &text, -1);
            if (text)
            {
                menu->GetVariables()[selection]->val = text;
                menu->LuaDataChanged();
                g_free(text);
            }
        }
    }
}
