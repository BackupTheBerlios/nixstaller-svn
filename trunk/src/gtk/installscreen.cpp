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
#include "installscreen.h"
#include "luagroup.h"

// -------------------------------------
// GTK Install Screen Class
// -------------------------------------

CInstallScreen::CInstallScreen(const std::string &title, CInstaller *owner) : CBaseScreen(title), m_pOwner(owner),
                                                                              m_WidgetRange(NULL, NULL)
{
    m_pMainBox = gtk_vbox_new(FALSE, 0);
    
    GtkWidget *box = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(box);
    gtk_box_pack_start(GTK_BOX(m_pMainBox), box, FALSE, FALSE, 5);
    
    m_pCounter = gtk_label_new(NULL);
    gtk_box_pack_end(GTK_BOX(box), m_pCounter, FALSE, FALSE, 10);
    
    m_pGroupBox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(m_pGroupBox);
    gtk_container_add(GTK_CONTAINER(m_pMainBox), m_pGroupBox);
}

CBaseLuaGroup *CInstallScreen::CreateGroup()
{
    CLuaGroup *ret = new CLuaGroup();
    gtk_box_pack_start(GTK_BOX(m_pGroupBox), ret->GetBox(), FALSE, FALSE, GroupSpacing());
    return ret;
}

void CInstallScreen::CoreUpdateLanguage(void)
{
    if (!GetTitle().empty())
        m_pOwner->SetTitle(GetTranslation(GetTitle()));
}

int CInstallScreen::GetTotalWidgetH(GtkWidget *w)
{
    GtkRequisition req;
    gtk_widget_size_request(w, &req);
    return (req.height + (GroupSpacing()*2));
}

void CInstallScreen::ResetWidgetRange()
{
    m_WidgetRange.first = m_WidgetRange.second = NULL;
    
    GList *list = gtk_container_get_children(GTK_CONTAINER(m_pGroupBox));
    GList *entry = list;
        
    if (!list)
        return;
        
    int h = 0;
    for (; entry; entry=g_list_next(entry))
    {
        h += GetTotalWidgetH(GTK_WIDGET(entry->data));

        if (h < MaxScreenHeight())
        {
            gtk_widget_show(GTK_WIDGET(entry->data));
            m_WidgetRange.second = GTK_WIDGET(entry->data);
            continue;
        }
        
        gtk_widget_hide(GTK_WIDGET(entry->data));
    }
    
    g_list_free(list);

    UpdateCounter();
}

void CInstallScreen::UpdateCounter()
{
    GList *list = gtk_container_get_children(GTK_CONTAINER(m_pGroupBox)), *entry = list;
    
    if (!list)
        return;
    
    int count = 1, current = 1, h = 0;
    
    for (; entry; entry=g_list_next(entry))
    {
        const int wh = GetTotalWidgetH(GTK_WIDGET(entry->data));
        
        if ((h + wh) >= MaxScreenHeight())
        {
            h = 0;
            count++;
        }
        
        h += wh;

        if (GTK_WIDGET(entry->data) == m_WidgetRange.first)
            current = count;
    }
    
    if (count > 1)
    {
        gtk_label_set(GTK_LABEL(m_pCounter), CreateText("%d/%d", current, count));
        gtk_widget_show(m_pCounter);
    }
    else
        gtk_widget_hide(m_pCounter);
    
    g_list_free(list);
}

void CInstallScreen::CoreActivate(void)
{
    ResetWidgetRange();
    if (!GetTitle().empty())
        m_pOwner->SetTitle(GetTranslation(GetTitle()));
    CBaseScreen::CoreActivate();
}

bool CInstallScreen::HasPrevWidgets() const
{
    if (!m_WidgetRange.first)
        return false;
    
    GList *list = gtk_container_get_children(GTK_CONTAINER(m_pGroupBox));
    
    if (!list)
        return false;
    
    bool ret = (GTK_WIDGET(list->data) != m_WidgetRange.first);
    g_list_free(list);
    
    return ret;
}

bool CInstallScreen::HasNextWidgets() const
{
    if (!m_WidgetRange.second)
        return false;

    GList *list = gtk_container_get_children(GTK_CONTAINER(m_pGroupBox));
    
    if (!list)
        return false;
    
    bool ret = (GTK_WIDGET(g_list_last(list)->data) != m_WidgetRange.second);
    g_list_free(list);
    
    return ret;
}

bool CInstallScreen::SubBack()
{
    if (HasPrevWidgets())
    {
        GList *list = gtk_container_get_children(GTK_CONTAINER(m_pGroupBox));
        GList *entry = list;
        
        if (!list)
            return false;
        
        for (; entry; entry=g_list_next(entry))
            gtk_widget_hide(GTK_WIDGET(entry->data));
        
        int h = 0;
        entry = g_list_find(list, m_WidgetRange.first);
        m_WidgetRange.first = m_WidgetRange.second = NULL;
        
        if (entry)
        {
            while ((entry = g_list_previous(entry)) && (h < MaxScreenHeight()))
            {
                h += GetTotalWidgetH(GTK_WIDGET(entry->data));
                
                if (h < MaxScreenHeight())
                {
                    gtk_widget_show(GTK_WIDGET(entry->data));
                    m_WidgetRange.first = GTK_WIDGET(entry->data);
                }
                
                if (!m_WidgetRange.second)
                    m_WidgetRange.second = GTK_WIDGET(entry->data);
            }
        }
        
        g_list_free(list);

        if (h)
        {
            UpdateCounter();
            return true;
        }
    }
    
    return false;
}

bool CInstallScreen::SubNext()
{
    if (HasNextWidgets())
    {
        GList *list = gtk_container_get_children(GTK_CONTAINER(m_pGroupBox));
        GList *entry = list;
        
        if (!list)
            return false;
        
        for (; entry; entry=g_list_next(entry))
            gtk_widget_hide(GTK_WIDGET(entry->data));
        
        int h = 0;
        entry = g_list_find(list, m_WidgetRange.second);
        m_WidgetRange.first = m_WidgetRange.second = NULL;
        
        if (entry)
        {
            while ((entry = g_list_next(entry)) && (h < MaxScreenHeight()))
            {
                h += GetTotalWidgetH(GTK_WIDGET(entry->data));
                
                if (h < MaxScreenHeight())
                {
                    gtk_widget_show(GTK_WIDGET(entry->data));
                    m_WidgetRange.second = GTK_WIDGET(entry->data);
                }
                
                if (!m_WidgetRange.first)
                    m_WidgetRange.first = GTK_WIDGET(entry->data);
            }
        }
        
        g_list_free(list);

        if (h)
        {
            UpdateCounter();
            return true;
        }
    }
    
    return false;
}

