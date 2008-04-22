/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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
class CLuaGroup;
class Fl_Group;
class Fl_Pack;
class Fl_Widget;

class CInstallScreen: public CBaseScreen
{
    Fl_Pack *m_pMainPack, *m_pWidgetPack;
    Fl_Group *m_pWidgetGroup;
    Fl_Box *m_pCounter;
    TSTLVecSize m_CurSubScreen;
    std::vector<int> m_WidgetRanges;
    int m_iMaxHeight;

    virtual CBaseLuaGroup *CreateGroup(void);
    virtual void CoreUpdateLanguage(void) {}
    
    int WidgetHSpacing(void) const { return 10; }

    CLuaGroup *GetGroup(Fl_Widget *w);
    int CheckTotalWidgetH(Fl_Widget *w);
    void ResetWidgetRange(void);
    void UpdateCounter(void);
    bool CheckWidgets(void);
    void ActivateSubScreen(TSTLVecSize screen);

protected:
    virtual void CoreActivate(void);
    
public:
    CInstallScreen(const std::string &title);
    virtual ~CInstallScreen(void) { DeleteGroups(); }
    
    using CBaseScreen::GetTitle; // So CInstallScreen can use it
    Fl_Group *GetGroup(void);
    
    void SetSize(int x, int y, int w, int h);
    
    bool HasPrevWidgets(void) const;
    bool HasNextWidgets(void) const;
    bool SubBack(void);
    bool SubNext(bool check=true);
    void SubLast(void);
};

#endif

