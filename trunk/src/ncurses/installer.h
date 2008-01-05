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

#ifndef INSTALLER_H
#define INSTALLER_H

#include <vector>
#include "ncurses.h"

namespace NNCurses {

class CButton;
class CWidget;

}

class CInstallScreen;

class CInstaller: public CNCursBase, public CBaseInstall
{
    typedef std::vector<CInstallScreen *> TScreenList;
    NNCurses::CButton *m_pCancelButton, *m_pPrevButton, *m_pNextButton;
    NNCurses::CBox *m_pScreenBox, *m_pButtonBox;
    TScreenList m_InstallScreens;
    TSTLVecSize m_CurrentScreen;
    bool m_bPrevButtonLocked;
    
    bool FirstValidScreen(void);
    bool LastValidScreen(void);
    void UpdateButtons(void);
    void PrevScreen(void);
    void NextScreen(void);
    void AskQuit(void);
    void ActivateScreen(CInstallScreen *screen, bool sublast=false);
    
    virtual CBaseScreen *CreateScreen(const std::string &title);
    virtual void CoreAddScreen(CBaseScreen *screen);
    virtual void CoreUpdateUI(void);
    virtual void LockScreen(bool cancel, bool prev, bool next);

protected:
    virtual void InitLua(void);
    virtual void CoreUpdateLanguage(void);
    virtual bool CoreHandleEvent(NNCurses::CWidget *emitter, int type);
    virtual bool CoreHandleKey(chtype key);

public:
    CInstaller(void);
    
    virtual void Init(int argc, char **argv);
};



#endif
