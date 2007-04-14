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

#ifndef INSTALLER
#define INSTALLER

#include <vector>
#include "main.h"
#include "ncurses.h"

namespace NNCurses {

class CButton;
class CWidget;

}

class CInstaller: public CNCursBase, public CBaseInstall
{
    NNCurses::CButton *m_pCancelButton, *m_pPrevButton, *m_pNextButton;
    NNCurses::CBox *m_pScreenBox;
//     std::vector<CBaseScreen *> m_InstallScreens;
    TSTLVecSize m_CurrentScreen;
    
    void PrevScreen(void){}
    void NextScreen(void){}
    void AskQuit(void);
    
protected:
    virtual void ChangeStatusText(const char *str) { }
    virtual void AddInstOutput(const std::string &str) { }
    virtual void SetProgress(int percent) { }
    virtual void InstallThink(void);

    virtual bool CoreHandleEvent(NNCurses::CWidget *emitter, int type);
    virtual bool CoreHandleKey(NNCurses::chtype key);

    virtual void InitLua(void);

public:
    CInstaller(void);
    
    virtual void Init(int argc, char **argv);
    virtual void UpdateLanguage(void) {}
    virtual void Install(void) {}
    virtual CBaseCFGScreen *CreateCFGScreen(const char *title) {}
};

#endif
