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

class CInstaller;
class CBaseLuaGroup;

class CInstallScreen: public CBaseScreen
{
    CInstaller *m_pOwner;
    GtkWidget *m_pMainBox, *m_pCounter, *m_pGroupBox;
    std::pair<GtkWidget *, GtkWidget *> m_WidgetRange;

    virtual CBaseLuaGroup *CreateGroup(void);
    virtual void CoreUpdateLanguage(void) {}
    
    int CheckTotalWidgetH(GtkWidget *w);
    void ResetWidgetRange(void);
    void UpdateCounter(void);
    bool CheckWidgets(void);

protected:
    virtual void CoreActivate(void);
    
public:
    CInstallScreen(const std::string &title, CInstaller *owner);
    virtual ~CInstallScreen(void) { DeleteGroups(); }
    
    void Init(void) { ResetWidgetRange(); }
    GtkWidget *GetBox(void) { return m_pMainBox; }
    
    bool HasPrevWidgets(void) const;
    bool HasNextWidgets(void) const;
    bool SubBack(void);
    bool SubNext(bool check=true);
    void SubLast(void);
};

#endif

