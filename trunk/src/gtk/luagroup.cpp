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

#include "main/main.h"
#include "gtk.h"
#include "luagroup.h"
#include "luacfgmenu.h"
// #include "luacheckbox.h"
// #include "luadirselector.h"
// #include "luaimage.h"
// #include "luainput.h"
#include "lualabel.h"
#include "luamenu.h"
// #include "luaprogressbar.h"
// #include "luaradiobutton.h"
// #include "luatextfield.h"

// -------------------------------------
// Lua Widget Group Class
// -------------------------------------

CLuaGroup::CLuaGroup()
{
    m_pBox = gtk_hbox_new(FALSE, 10);
    gtk_widget_show(m_pBox);
}

CBaseLuaCFGMenu *CLuaGroup::CreateCFGMenu(const char *desc)
{
    CLuaCFGMenu *ret = new CLuaCFGMenu(desc);
    AddWidget(ret);
    return ret;
}

CBaseLuaMenu *CLuaGroup::CreateMenu(const char *desc, const std::vector<std::string> &l)
{
    CLuaMenu *ret = new CLuaMenu(desc, l);
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
    gtk_widget_show(w->GetBox());
    gtk_box_pack_start(GTK_BOX(m_pBox), w->GetBox(), TRUE, TRUE, 10);
}
