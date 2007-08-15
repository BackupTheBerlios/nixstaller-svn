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
                               m_pNextSubScreen(NULL)
{
    m_pMainBox = gtk_vbox_new(FALSE, 0);
}

CBaseLuaGroup *CInstallScreen::CreateGroup()
{
    CLuaGroup *ret = new CLuaGroup();
    gtk_widget_show(ret->GetBox());
    gtk_box_pack_start(GTK_BOX(m_pMainBox), ret->GetBox(), FALSE, FALSE, 10);
    return ret;
}

void CInstallScreen::CoreUpdateLanguage(void)
{
    if (!GetTitle().empty())
        m_pOwner->SetTitle(GetTranslation(GetTitle()));
}

void CInstallScreen::CoreActivate(void)
{
    if (!GetTitle().empty())
        m_pOwner->SetTitle(GetTranslation(GetTitle()));
    CBaseScreen::CoreActivate();
}
