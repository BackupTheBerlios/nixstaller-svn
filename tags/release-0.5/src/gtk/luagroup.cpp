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

#include "main/main.h"
#include "gtk.h"
#include "installscreen.h"
#include "luagroup.h"
#include "luacfgmenu.h"
#include "luacheckbox.h"
#include "luadirselector.h"
#include "luaimage.h"
#include "luainput.h"
#include "lualabel.h"
#include "luamenu.h"
#include "luaprogressbar.h"
#include "luaradiobutton.h"
#include "luatextfield.h"

// -------------------------------------
// Lua Widget Group Class
// -------------------------------------

CLuaGroup::CLuaGroup(CInstallScreen *screen) : m_pInstallScreen(screen)
{
    m_pBox = gtk_hbox_new(FALSE, 10);
    gtk_widget_show(m_pBox);
}

CBaseLuaInputField *CLuaGroup::CreateInputField(const char *label, const char *desc, const char *val,
                                                int max, const char *type)
{
    CLuaInputField *ret = new CLuaInputField(label, desc, val, max, type);
    AddWidget(ret);
    return ret;
}

CBaseLuaCheckbox *CLuaGroup::CreateCheckbox(const char *desc, const std::vector<std::string> &l,
                                            const std::vector<TSTLVecSize> &e)
{
    CLuaCheckbox *ret = new CLuaCheckbox(desc, l, e);
    AddWidget(ret);
    return ret;
}

CBaseLuaRadioButton *CLuaGroup::CreateRadioButton(const char *desc, const std::vector<std::string> &l,
                                                  TSTLVecSize e)
{
    CLuaRadioButton *ret = new CLuaRadioButton(desc, l, e);
    AddWidget(ret);
    return ret;
}

CBaseLuaDirSelector *CLuaGroup::CreateDirSelector(const char *desc, const char *val)
{
    CLuaDirSelector *ret = new CLuaDirSelector(desc, val);
    AddWidget(ret);
    return ret;
}

CBaseLuaCFGMenu *CLuaGroup::CreateCFGMenu(const char *desc)
{
    CLuaCFGMenu *ret = new CLuaCFGMenu(desc);
    AddWidget(ret);
    return ret;
}

CBaseLuaMenu *CLuaGroup::CreateMenu(const char *desc, const std::vector<std::string> &l,
                                    TSTLVecSize e)
{
    CLuaMenu *ret = new CLuaMenu(desc, l, e);
    AddWidget(ret);
    return ret;
}

CBaseLuaImage *CLuaGroup::CreateImage(const char *file)
{
    CLuaImage *ret = new CLuaImage(file);
    if (ret->HasValidImage())
    {
        gtk_widget_show(ret->GetBox());
        gtk_box_pack_start(GTK_BOX(m_pBox), ret->GetBox(), FALSE, FALSE, 10);
    }
    return ret;
}

CBaseLuaProgressBar *CLuaGroup::CreateProgressBar(const char *desc)
{
    CLuaProgressBar *ret = new CLuaProgressBar(desc);
    AddWidget(ret);
    return ret;
}

CBaseLuaTextField *CLuaGroup::CreateTextField(const char *desc, bool wrap, const char *size)
{
    CLuaTextField *ret = new CLuaTextField(desc, wrap, size);
    AddWidget(ret);
    return ret;
}

CBaseLuaLabel *CLuaGroup::CreateLabel(const char *title)
{
    CLuaLabel *ret = new CLuaLabel(title);
    AddWidget(ret);
    return ret;
}

void CLuaGroup::AddWidget(CLuaWidget *w)
{
    w->SetScreen(m_pInstallScreen);
    
    gtk_widget_show(w->GetBox());
    
    GtkWidget *box = gtk_vbox_new(FALSE, 0);
    gtk_object_set_user_data(GTK_OBJECT(box), w);
    gtk_widget_show(box);
    gtk_box_pack_start(GTK_BOX(box), w->GetBox(), TRUE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(m_pBox), box, TRUE, TRUE, 10);
    
    const int size = ContainerSize(GTK_CONTAINER(m_pBox));
    const int maxw = (MaxWidgetWidth() - ((size-1) * gtk_box_get_spacing(GTK_BOX(m_pBox)))) / size;
    CPointerWrapper<GList> list(gtk_container_get_children(GTK_CONTAINER(m_pBox)), g_list_free);
    GList *entry = list;
    
    if (list)
    {
        for (; entry; entry=g_list_next(entry))
        {
            void *udata = gtk_object_get_user_data(GTK_OBJECT(entry->data));
            if (udata) // LuaImage doesn't set this
            {
                CLuaWidget *lw = static_cast<CLuaWidget *>(udata);
                lw->SetMaxWidth(maxw);
            }
        }
    }
}
