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
#include "luainput.h"

// -------------------------------------
// Lua GTK InputField Class
// -------------------------------------

CLuaInputField::CLuaInputField(const char *label, const char *desc, const char *val, int max,
                               const char *type) : CBaseLuaWidget(desc), CBaseLuaInputField(label, type), m_pLabel(NULL)
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(GetBox()), hbox, TRUE, FALSE, 0);
    
    if (label && *label)
    {
        GtkWidget *labelbox = gtk_vbox_new(FALSE, 0);
        m_pLabel = gtk_label_new(NULL); // Text will be set in SetLabel()
        SetLabel();
        gtk_box_pack_start(GTK_BOX(labelbox), m_pLabel, TRUE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), labelbox, FALSE, FALSE, 0);
    }
    
    m_pEntry = gtk_entry_new();
    g_signal_connect(G_OBJECT(m_pEntry), "insert_text", G_CALLBACK(InsertCB), const_cast<char *>(type));
    g_signal_connect(G_OBJECT(m_pEntry), "changed", G_CALLBACK(InputChangedCB), this);
    
    if (val && *val)
        gtk_entry_set_text(GTK_ENTRY(m_pEntry), val);
    
    gtk_entry_set_max_length(GTK_ENTRY(m_pEntry), max);
    gtk_container_add(GTK_CONTAINER(hbox), m_pEntry);
    
    gtk_widget_show_all(hbox);
}

const char *CLuaInputField::CoreGetValue(void)
{
    return gtk_entry_get_text(GTK_ENTRY(m_pEntry));
}

void CLuaInputField::CoreUpdateLanguage()
{
    if (m_pLabel)
        SetLabel();
}

void CLuaInputField::CoreUpdateLabelWidth()
{
    if (m_pLabel)
        SetLabel();
}

void CLuaInputField::CoreActivateWidget()
{
    gtk_widget_grab_focus(m_pEntry);
}

void CLuaInputField::SetLabel()
{
    if (GetLabel().empty())
        return;
    
    TSTLStrSize max = SafeConvert<TSTLStrSize>(GetLabelWidth());
    if (GetLabel().length() > max)
    {
        std::string label = GetTranslation(GetLabel()).substr(0, max);
        gtk_label_set(GTK_LABEL(m_pLabel), label.c_str());
    }
    else
    {
        std::string label = GetTranslation(GetLabel());
        label.append(max - GetLabel().length(), ' ');
        gtk_label_set(GTK_LABEL(m_pLabel), label.c_str());
    }
}

void CLuaInputField::InsertCB(GtkEditable *editable, gchar *nt, gint new_text_length, gint *position,
                              gpointer data)
{
    const char *type = static_cast<char *>(data);
    
    if (!strcmp(type, "number") || !strcmp(type, "float"))
    {
        lconv *lc = localeconv();
        if (strchr(lc->decimal_point, ','))
        {
            // Convert '.' to ','
            gchar *c = nt;
            while (c && *c)
            {
                if (*c == '.')
                    *c = ',';
                c++;
            }
        }

        std::string curtext = gtk_entry_get_text(GTK_ENTRY(editable));
        std::string legal = LegalNrTokens(!strcmp(type, "float"), curtext, *position);
        
        if (strpbrk(nt, legal.c_str()) == NULL)
            g_signal_stop_emission(editable, g_signal_lookup("insert_text", g_type_from_name("GtkEditable")), 0);
    }
}

void CLuaInputField::InputChangedCB(GtkEditable *widget, gpointer data)
{
    CLuaInputField *input = static_cast<CLuaInputField *>(data);
    input->LuaDataChanged();
}
