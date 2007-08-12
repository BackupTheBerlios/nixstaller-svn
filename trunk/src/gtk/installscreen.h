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

#ifndef INSTALL_SCREEN_H
#define INSTALL_SCREEN_H

#include "main/install/basescreen.h"

class CBaseLuaGroup;

class CInstallScreen: public CBaseScreen
{
    GtkWidget *m_pTitle, *m_pCounter, *m_pMainBox, *m_pTopBox, *m_pGroupBox;
    CInstallScreen *m_pNextSubScreen;

    virtual CBaseLuaGroup *CreateGroup(void);
    virtual void CoreUpdateLanguage(void);
    
public:
    CInstallScreen(const std::string &title);
    
    GtkWidget *GetBox(void) { return m_pMainBox; }
    CInstallScreen *GetNextSubScreen(void) { return m_pNextSubScreen; }
};

#endif

