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

#include "fltk.h"

class CInstallScreen;
class Fl_Box;
class Fl_Button;
class Fl_Group;
class Fl_Pack;
class Fl_Widget;
class Fl_Window;
class Fl_Wizard;

class CInstaller: public CFLTKBase, public CBaseInstall
{
    Fl_Window *m_pMainWindow;
    Fl_Group *m_pHeaderGroup;
    Fl_Box *m_pTitle, *m_pLogoBox, *m_pAboutBox;
    Fl_Button *m_pCancelButton, *m_pBackButton, *m_pNextButton;
    Fl_Pack *m_pButtonPack; // pack for back and next buttons
    Fl_Wizard *m_pWizard;
    bool m_bPrevButtonLocked;
    std::string m_CurTitle;
    
    int WindowW(void) const { return 600; }
    int WindowH(void) const { return 400; }
    int HeaderSpacing(void) const { return 5; }
    int ScreenSpacing(void) const { return 10; }
    int ButtonWOffset(void) const { return 10; }
    int ButtonWSpacing(void) const { return 10; }
    int ButtonHSpacing(void) const { return 10; }
    
    void CreateHeader(void);
    void SetTitle(const std::string &t);
    
    CInstallScreen *GetScreen(Fl_Widget *w);
    void ActivateScreen(CInstallScreen *screen);
    bool FirstValidScreen(void);
    bool LastValidScreen(void);
    void UpdateButtonPack(void);
    void UpdateButtons(void);
    void Back(void);
    void Next(void);

    virtual CBaseScreen *CreateScreen(const std::string &title);
    virtual void CoreAddScreen(CBaseScreen *screen);
    virtual CBaseLuaProgressDialog *CoreCreateProgDialog(const std::vector<std::string> &l, int r) { }
    virtual void CoreUpdateUI(void);
    virtual void LockScreen(bool cancel, bool prev, bool next);
    virtual void CoreUpdateLanguage(void);

public:
    CInstaller(void) : m_pLogoBox(NULL), m_bPrevButtonLocked(false) {}
    virtual ~CInstaller(void) { DeleteScreens(); }
    
    virtual void Init(int argc, char **argv);
    
    static void CancelCB(Fl_Widget *w, void *p);
    static void BackCB(Fl_Widget *w, void *p) { ((CInstaller *)p)->Back(); };
    static void NextCB(Fl_Widget *w, void *p) { ((CInstaller *)p)->Next(); };
};

#endif
