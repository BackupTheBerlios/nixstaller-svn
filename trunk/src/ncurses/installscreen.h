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

#ifndef INSTALL_SCREEN_H
#define INSTALL_SCREEN_H

#include "main/install/basescreen.h"
#include "tui/tui.h"
#include "tui/box.h"

namespace NNCurses {
class CLabel;
}

class CBaseLuaGroup;

class CInstallScreen: public CBaseScreen, public NNCurses::CBox
{
    NNCurses::CLabel *m_pTitle, *m_pCounter;
    NNCurses::CBox *m_pTopBox, *m_pGroupBox;
    std::pair<NNCurses::CWidget *, NNCurses::CWidget *> m_WidgetRange;
    
    virtual CBaseLuaGroup *CreateGroup(void);
    virtual void CoreUpdateLanguage(void);
    
    void ResetWidgetRange(void);
    int MaxScreenHeight(void) const { return NNCurses::GetMaxHeight() - 7; }
    int StartingHeight(void);
    void UpdateCounter(void);
    
protected:
/*    virtual bool CoreBack(void);
    virtual bool CoreNext(void);*/
    virtual void CoreActivate(void) { ResetWidgetRange(); CBaseScreen::CoreActivate(); }
    virtual int CoreRequestHeight(void);
    virtual void CoreInit(void) { ResetWidgetRange(); }
    
public:
    CInstallScreen(const std::string &title);
    
    bool HasPrevWidgets(void) const { return (m_pGroupBox && (m_WidgetRange.first != NULL)); }
    bool HasNextWidgets(void) { return (m_pGroupBox && (m_WidgetRange.second != m_pGroupBox->GetChildList().back())); }
    bool SubNext(void);
    bool SubBack(void);
};

#endif
