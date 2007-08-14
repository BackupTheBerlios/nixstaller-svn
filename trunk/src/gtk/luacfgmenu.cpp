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

CLuaCFGMenu::CLuaCFGMenu(const char *desc) : CBaseLuaWidget(desc)
{
    gtk_box_set_spacing(GTK_BOX(GetBox()), 15);
    
    GtkWidget *varbox = CreateVarListBox();
    gtk_container_add(GTK_CONTAINER(GetBox()), varbox);
    
    GtkWidget *valbox = gtk_hbox_new(FALSE, 15);
    
    GtkWidget *label = gtk_label_new("Value:");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(valbox), label, FALSE, FALSE, 5);
    
    m_pInputField = CreateInputField();
    gtk_box_pack_start(GTK_BOX(valbox), m_pInputField, TRUE, TRUE, 15);
    
    m_pComboBox = CreateComboBox();
    gtk_box_pack_start(GTK_BOX(valbox), m_pComboBox, TRUE, TRUE, 15);
    
    m_pDirInputBox = CreateDirSelector();
    gtk_box_pack_start(GTK_BOX(valbox), m_pDirInputBox, TRUE, TRUE, 15);
    
    gtk_widget_show(valbox);
    gtk_container_add(GTK_CONTAINER(GetBox()), valbox);
}

GtkWidget *CLuaCFGMenu::CreateVarListBox()
{
    // Add horiz box for scroll window, so it can have horizontal spacing
    GtkWidget *swbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(swbox);
    
    // Scrolling window for treeview
    GtkWidget *sw = gtk_scrolled_window_new(0, 0);
    gtk_widget_set_size_request(sw, -1, 175);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(swbox), sw, TRUE, TRUE, 15);
    
    GtkListStore *store = gtk_list_store_new(COLUMN_N, G_TYPE_STRING, G_TYPE_STRING);
    
    m_pVarListView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    
    g_object_unref(store);
    
    GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pVarListView));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(select), "changed", G_CALLBACK(SelectionCB), this);
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Variable", renderer, "text", COLUMN_VAR, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_pVarListView), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(GTK_OBJECT(renderer), "wrap-width", MaxWidgetReqW()-100);
    column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", COLUMN_DESC, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_pVarListView), column);

    gtk_container_add(GTK_CONTAINER(sw), m_pVarListView);
    gtk_widget_show_all(sw);
    
    return swbox;
}

GtkWidget *CLuaCFGMenu::CreateInputField()
{
    return gtk_entry_new();
}

GtkWidget *CLuaCFGMenu::CreateComboBox()
{
    GtkWidget *combo = gtk_combo_box_new_text();
    return combo;
}

GtkWidget *CLuaCFGMenu::CreateDirSelector()
{
    GtkWidget *box = gtk_hbox_new(FALSE, 10);
    
    m_pDirInput = gtk_entry_new();
    gtk_widget_show(m_pDirInput);
    gtk_container_add(GTK_CONTAINER(box), m_pDirInput);
    
    GtkWidget *button = CreateButton(m_pDirButtonLabel = gtk_label_new(GetTranslation("Browse")), GTK_STOCK_OPEN);
    gtk_widget_show(button);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
    
    return box;
}

void CLuaCFGMenu::UpdateSelection(gchar *var)
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
            gtk_combo_box_append_text(GTK_COMBO_BOX(m_pComboBox), "Test 1");
            gtk_widget_show(m_pComboBox);
            break;
    }
}

void CLuaCFGMenu::CoreAddVar(const char *name)
{
    GtkTreeIter iter;
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(m_pVarListView)));
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, name, 1, GetVariables()[name]->desc.c_str(), -1);
}

void CLuaCFGMenu::CoreUpdateLanguage()
{
    // UNDONE
}

void CLuaCFGMenu::SelectionCB(GtkTreeSelection *selection, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *var;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_tree_model_get (model, &iter, COLUMN_VAR, &var, -1);
        
        CLuaCFGMenu *menu = static_cast<CLuaCFGMenu *>(data);
        menu->UpdateSelection(var);

        g_free(var);
    }
}

