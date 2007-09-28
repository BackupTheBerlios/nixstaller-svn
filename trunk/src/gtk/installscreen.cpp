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
    gtk_box_pack_start(GTK_BOX(m_pMainBox), box, FALSE, FALSE, 0);
    
    m_pCounter = gtk_label_new(NULL);
    gtk_box_pack_end(GTK_BOX(box), m_pCounter, FALSE, FALSE, 10);
    
    m_pGroupBox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(m_pGroupBox);
    gtk_container_add(GTK_CONTAINER(m_pMainBox), m_pGroupBox);
}

CBaseLuaGroup *CInstallScreen::CreateGroup()
{
    CLuaGroup *ret = new CLuaGroup();
    gtk_object_set_user_data(GTK_OBJECT(ret->GetBox()), ret);
    gtk_box_pack_start(GTK_BOX(m_pGroupBox), ret->GetBox(), TRUE, FALSE, WidgetGroupSpacing());
    gtk_widget_hide(ret->GetBox());
    return ret;
}

int CInstallScreen::CheckTotalWidgetH(GtkWidget *w)
{
    GtkRequisition req;
    gtk_widget_size_request(w, &req);
    
    int ret = (req.height + (WidgetGroupSpacing()*2));
    
    if (ret > MaxScreenHeight())
        throw Exceptions::CExOverflow("Not enough space for widget.");
    
    return ret;
}

void CInstallScreen::ResetWidgetRange()
{
    m_WidgetRange.first = m_WidgetRange.second = NULL;
    
    CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(m_pGroupBox)), g_list_free);
    GList *entry = list;
        
    if (!list)
        return;
        
    int h = 0;
    for (; entry; entry=g_list_next(entry))
    {
        if (ContainerEmpty(GTK_CONTAINER(GTK_WIDGET(entry->data))))
            continue;
        
        h += CheckTotalWidgetH(GTK_WIDGET(entry->data));

        if (h <= MaxScreenHeight())
        {
            gtk_widget_show(GTK_WIDGET(entry->data));
            m_WidgetRange.second = GTK_WIDGET(entry->data);
            continue;
        }
        
        gtk_widget_hide(GTK_WIDGET(entry->data));
    }
    
    UpdateCounter();
}

void CInstallScreen::UpdateCounter()
{
    CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(m_pGroupBox)), g_list_free);
    GList *entry = list;
    
    if (!list)
        return;
    
    int count = 1, current = 1, h = 0;
    
    for (; entry; entry=g_list_next(entry))
    {
        if (ContainerEmpty(GTK_CONTAINER(entry->data)))
            continue;

        const int wh = CheckTotalWidgetH(GTK_WIDGET(entry->data));
        
        if ((h + wh) > MaxScreenHeight())
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
}

void CInstallScreen::CoreActivate(void)
{
    ResetWidgetRange();
    m_pOwner->SetTitle(GetTitle());
    CBaseScreen::CoreActivate();
}

bool CInstallScreen::CheckWidgets()
{
    CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(m_pGroupBox)), g_list_free);
    
    if (!list)
        return true;
    
    GList *start = (m_WidgetRange.first) ? g_list_find(list, m_WidgetRange.first) : &(*list);
    GList *end = (m_WidgetRange.second) ? g_list_find(list, m_WidgetRange.second) : g_list_last(list);
    bool ret = true;
    
    while (true)
    {
        CLuaGroup *group = static_cast<CLuaGroup *>(gtk_object_get_user_data(GTK_OBJECT(start->data)));
        if (!group->CheckWidgets())
        {
            ret = false;
            break;
        }
        
        if (start == end)
            break;
        
        start = g_list_next(start);
    }

    return ret;
}

bool CInstallScreen::HasPrevWidgets() const
{
    if (!m_WidgetRange.first)
        return false;
    
    CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(m_pGroupBox)), g_list_free);
    
    if (!list)
        return false;
    
    bool ret = (GTK_WIDGET(list->data) != m_WidgetRange.first);
    
    return ret;
}

bool CInstallScreen::HasNextWidgets() const
{
    if (!m_WidgetRange.second)
        return false;

    CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(m_pGroupBox)), g_list_free);
    
    if (!list)
        return false;
    
    bool ret = (GTK_WIDGET(g_list_last(list)->data) != m_WidgetRange.second);
    
    return ret;
}

bool CInstallScreen::SubBack()
{
    if (HasPrevWidgets())
    {
        CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(m_pGroupBox)), g_list_free);
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
            while ((entry = g_list_previous(entry)) && (h <= MaxScreenHeight()))
            {
                if (ContainerEmpty(GTK_CONTAINER(entry->data)))
                    continue;

                h += CheckTotalWidgetH(GTK_WIDGET(entry->data));
                
                if (h <= MaxScreenHeight())
                {
                    gtk_widget_show(GTK_WIDGET(entry->data));
                    m_WidgetRange.first = GTK_WIDGET(entry->data);
                }
                
                if (!m_WidgetRange.second)
                    m_WidgetRange.second = GTK_WIDGET(entry->data);
            }
        }
        
        if (h)
        {
            UpdateCounter();
            return true;
        }
    }
    
    return false;
}

bool CInstallScreen::SubNext(bool check)
{
    if (check && !CheckWidgets())
        return true; // Widget check failed, so return true in order to stay at this screen
    
    if (HasNextWidgets())
    {
        CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(m_pGroupBox)), g_list_free);
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
            while ((entry = g_list_next(entry)) && (h <= MaxScreenHeight()))
            {
                if (ContainerEmpty(GTK_CONTAINER(entry->data)))
                    continue;

                h += CheckTotalWidgetH(GTK_WIDGET(entry->data));
                
                if (h <= MaxScreenHeight())
                {
                    gtk_widget_show(GTK_WIDGET(entry->data));
                    m_WidgetRange.second = GTK_WIDGET(entry->data);
                }
                
                if (!m_WidgetRange.first)
                    m_WidgetRange.first = GTK_WIDGET(entry->data);
            }
        }
        
        if (h)
        {
            UpdateCounter();
            return true;
        }
    }
    
    return false;
}

void CInstallScreen::SubLast()
{
    while (SubNext(false))
        ;
}
