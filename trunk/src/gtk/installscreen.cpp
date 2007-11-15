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

CInstallScreen::CInstallScreen(const std::string &title, CInstaller *owner) : CBaseScreen(title),
                                                                              m_pOwner(owner),
                                                                              m_CurSubScreen(0)
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
    m_WidgetRanges.clear();
    m_CurSubScreen = 0;
    
    CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(m_pGroupBox)), g_list_free);
    GList *entry = list;
        
    if (!list)
        return;
        
    m_WidgetRanges.push_back(GTK_WIDGET(entry->data));
    
    bool enablewidgets = true, endedscreen = false;
    int h = 0;
    for (; entry; entry=g_list_next(entry))
    {
        GtkWidget *w = GTK_WIDGET(entry->data);
        if (ContainerEmpty(GTK_CONTAINER(w)))
            continue;
        
        const int newh = CheckTotalWidgetH(w);

        if (!endedscreen && ((newh + h) <= MaxScreenHeight()))
        {
            h += newh;
            if (enablewidgets)
                gtk_widget_show(w);
            else
                gtk_widget_hide(w);
        }
        else
        {
            m_WidgetRanges.push_back(w);
            h = newh;
            enablewidgets = false;
            endedscreen = false;
            gtk_widget_hide(w);
        }
        CLuaGroup *lg = static_cast<CLuaGroup *>(gtk_object_get_user_data(GTK_OBJECT(w)));
        endedscreen = lg->EndsScreen();
    }
    
    UpdateCounter();
}

void CInstallScreen::UpdateCounter()
{
    TSTLVecSize size = m_WidgetRanges.size();
    if (size > 1)
    {
        gtk_label_set(GTK_LABEL(m_pCounter), CreateText("%d/%d", m_CurSubScreen+1, SafeConvert<int>(size)));
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
    
    GList *start, *end;

    if (HasPrevWidgets())
        start = g_list_find(list, m_WidgetRanges[m_CurSubScreen]);
    else
        start = &(*list);
    
    if (HasNextWidgets())
        end = g_list_find(list, m_WidgetRanges[m_CurSubScreen+1]);
    else
        end = NULL;
    
    bool ret = true;
    
    while (start && (start != end))
    {
        CLuaGroup *group = static_cast<CLuaGroup *>(gtk_object_get_user_data(GTK_OBJECT(start->data)));
        if (!group->CheckWidgets())
        {
            ret = false;
            break;
        }
        
        start = g_list_next(start);
    }

    return ret;
}

void CInstallScreen::ActivateSubScreen(TSTLVecSize screen)
{
    CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(m_pGroupBox)), g_list_free);
    GList *entry = list;
    
    for (; entry; entry=g_list_next(entry))
        gtk_widget_hide(GTK_WIDGET(entry->data));
    
    entry = g_list_find(list, m_WidgetRanges[screen]);
    
    GList *end;
    if ((m_WidgetRanges.size()-1) > screen)
        end = g_list_find(list, m_WidgetRanges[screen+1]);
    else
        end = NULL;
    
    while (entry && (entry != end))
    {
        gtk_widget_show(GTK_WIDGET(entry->data));
        entry = g_list_next(entry);
    }
    
    m_CurSubScreen = screen;
    UpdateCounter();
}

bool CInstallScreen::HasPrevWidgets() const
{
    return (!m_WidgetRanges.empty() && m_CurSubScreen);
}

bool CInstallScreen::HasNextWidgets() const
{
    return (!m_WidgetRanges.empty() && (m_CurSubScreen < (m_WidgetRanges.size()-1)));
}

bool CInstallScreen::SubBack()
{
    if (!HasPrevWidgets())
        return false;
    
    ActivateSubScreen(m_CurSubScreen - 1);
    return true;
}

bool CInstallScreen::SubNext(bool check)
{
    if (check && !CheckWidgets())
        return true; // Widget check failed, so return true in order to stay at this screen
    
    if (!HasNextWidgets())
        return false;
    
    ActivateSubScreen(m_CurSubScreen + 1);
    return true;
}

void CInstallScreen::SubLast()
{
    if (!m_WidgetRanges.empty())
        ActivateSubScreen(m_WidgetRanges.size()-1);
}
