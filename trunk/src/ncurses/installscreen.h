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
    TSTLVecSize m_CurSubScreen;
    std::vector<NNCurses::CWidget *> m_WidgetRanges;
    bool m_bDirty, m_bCleaning;
    
    virtual CBaseLuaGroup *CreateGroup(void);
    virtual void CoreUpdateLanguage(void);
    
    bool VisibleWidget(NNCurses::CWidget *w);
    int CheckWidgetHeight(NNCurses::CWidget *w);
    void ResetWidgetRange(void);
    int MaxScreenHeight(void) const;
    void UpdateCounter(void);
    bool CheckWidgets(void);
    void ActivateSubScreen(TSTLVecSize screen);
    
protected:
    virtual void CoreActivate(void);
    virtual void CoreInit(void) { ResetWidgetRange(); }
    virtual void CoreDraw(void);
    virtual bool CoreHandleEvent(CWidget *emitter, int event);
    
public:
    CInstallScreen(const std::string &title);
    
    bool HasPrevWidgets(void) const;
    bool HasNextWidgets(void) const;
    bool SubNext(bool check=true);
    bool SubBack(void);
    void SubLast(void);
};

#endif
