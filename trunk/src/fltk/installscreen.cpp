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

#include "fltk.h"
#include "installer.h"
#include "installscreen.h"
// #include "luagroup.h"
#include <FL/Fl_Group.H>

// -------------------------------------
// FLTK Install Screen Class
// -------------------------------------

CInstallScreen::CInstallScreen(const std::string &title, CInstaller *owner) : CBaseScreen(title),
                               m_pOwner(owner), m_WidgetRange(NULL, NULL)
{
    m_pMainGroup = new Fl_Group(0, 0, 0, 0); // Size and position is set when screen is activated
}

void CInstallScreen::CoreActivate(void)
{
//     ResetWidgetRange();
    if (!GetTitle().empty())
        m_pOwner->SetTitle(GetTitle());
    CBaseScreen::CoreActivate();
}
