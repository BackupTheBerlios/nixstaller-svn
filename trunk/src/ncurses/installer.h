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
#include "main/install/attinstall.h"
#include "ncurses.h"
#include "tui/window.h"

namespace NNCurses {

class CButton;
class CWidget;

}

class CInstallScreen;

class CInstaller: public NNCurses::CWindow, public CBaseAttInstall
{
    typedef std::vector<CInstallScreen *> TScreenList;
    NNCurses::CButton *m_pCancelButton, *m_pPrevButton, *m_pNextButton;
    NNCurses::CBox *m_pScreenBox, *m_pButtonBox;
    TScreenList m_InstallScreens;
    TSTLVecSize m_CurrentScreen;
    bool m_bPrevButtonLocked;
    
    void ShowAbout(void);
    bool FirstValidScreen(void);
    bool LastValidScreen(void);
    void UpdateButtons(void);
    void PrevScreen(void);
    void NextScreen(void);
    void ActivateScreen(CInstallScreen *screen, bool sublast=false);
    
    virtual char *GetPassword(const char *str);
    virtual void MsgBox(const char *str, ...);
    virtual bool YesNoBox(const char *str, ...);
    virtual int ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...);
    virtual void WarnBox(const char *str, ...);
    virtual void InitLua(void);
    virtual CBaseScreen *CreateScreen(const std::string &title);
    virtual void CoreAddScreen(CBaseScreen *screen);
    virtual CBaseLuaProgressDialog *CoreCreateProgDialog(int r);
    virtual CBaseLuaDepScreen *CoreCreateDepScreen(int f);
    virtual void CoreUpdate(void);
    virtual void LockScreen(bool cancel, bool prev, bool next);

protected:
    virtual void CoreUpdateLanguage(void);
    virtual bool CoreHandleEvent(NNCurses::CWidget *emitter, int type);
    virtual bool CoreHandleKey(chtype key);
    virtual void CoreGetButtonDescs(NNCurses::TButtonDescList &list);

public:
    CInstaller(void);
    
    virtual void Init(int argc, char **argv);
    
    void Run(void);
};

#endif
