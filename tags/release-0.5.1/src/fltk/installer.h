/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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

#include "main/install/attinstall.h"
#include "fltk.h"

class CInstallScreen;
class Fl_Box;
class Fl_Button;
class Fl_Group;
class Fl_Pack;
class Fl_Return_Button;
class Fl_Text_Display;
class Fl_Widget;
class Fl_Window;
class Fl_Wizard;

class CInstaller: public CBaseAttInstall
{
    Fl_Text_Display *m_pAboutDisp;
    Fl_Return_Button *m_pAboutOKButton;
    Fl_Window *m_pAboutWindow;
    Fl_Window *m_pMainWindow;
    Fl_Group *m_pHeaderGroup;
    Fl_Box *m_pTitle, *m_pLogoBox, *m_pAboutBox;
    Fl_Button *m_pCancelButton, *m_pBackButton, *m_pNextButton;
    Fl_Pack *m_pButtonPack; // pack for back and next buttons
    Fl_Wizard *m_pWizard;
    bool m_bPrevButtonLocked;
    std::string m_CurTitle;
    
    void CreateAbout(void);
    void ShowAbout(bool show);
    
    int WindowW(void) const { return 620; }
    int WindowH(void) const { return 420; }
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

    virtual char *GetPassword(const char *str);
    virtual void MsgBox(const char *str, ...);
    virtual bool YesNoBox(const char *str, ...);
    virtual int ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...);
    virtual void WarnBox(const char *str, ...);
    virtual int TextWidth(const char *str);
    virtual CBaseScreen *CreateScreen(const std::string &title);
    virtual void CoreAddScreen(CBaseScreen *screen);
    virtual CBaseLuaProgressDialog *CoreCreateProgDialog(int r);
    virtual CBaseLuaDepScreen *CoreCreateDepScreen(int f);
    virtual void CoreUpdate(void);
    virtual void LockScreen(bool cancel, bool prev, bool next);
    virtual void CoreUpdateLanguage(void);

protected:
    Fl_Window *GetMainWindow(void) { return m_pMainWindow; };

public:
    CInstaller(void);
    virtual ~CInstaller(void);
    
    virtual void Init(int argc, char **argv);

    void Run(void);
    void Cancel(void);

    static void TimedRunner(void *p);
    static void AboutOKCB(Fl_Widget *, void *p) { ((CInstaller *)p)->ShowAbout(false); };
    static void ShowAboutCB(Fl_Widget *, void *p) { ((CInstaller *)p)->ShowAbout(true); };
    static void CancelCB(Fl_Widget *w, void *p);
    static void BackCB(Fl_Widget *w, void *p);
    static void NextCB(Fl_Widget *w, void *p);
};

#endif
