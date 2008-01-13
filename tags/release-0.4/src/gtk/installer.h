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

class CInstallScreen;

class CInstaller: public CGTKBase, public CBaseInstall
{
    GtkWidget *m_pTitle, *m_pLogo, *m_pAboutLabel;
    GtkWidget *m_pCancelLabel, *m_pBackLabel, *m_pNextLabel;
    GtkWidget *m_pCancelButton, *m_pBackButton, *m_pNextButton;
    GtkWidget *m_pWizard;
    bool m_bPrevButtonLocked;
    std::string m_CurTitle;
    
    void SetAboutLabel(void);
    
    void InitAboutSection(GtkWidget *parentbox);
    void InitScreenSection(GtkWidget *parentbox);
    void InitButtonSection(GtkWidget *parentbox);
    
    CInstallScreen *GetScreen(gint index);
    
    bool FirstValidScreen(void);
    bool LastValidScreen(void);
    void UpdateButtons(void);
    void Back(void);
    void Next(void);

    bool AskQuit(void);
    
    int GetMainSpacing(void) const { return 10; }
    
    virtual void CoreUpdateUI(void);
    virtual CBaseScreen *CreateScreen(const std::string &title);
    virtual void CoreAddScreen(CBaseScreen *screen);
    virtual void LockScreen(bool cancel, bool prev, bool next);
    virtual void CoreUpdateLanguage(void);

public:
    CInstaller(void) : m_bPrevButtonLocked(false) {}
    virtual ~CInstaller(void) { DeleteScreens(); }
    
    virtual void Init(int argc, char **argv);
    
    void SetTitle(const std::string &t);
    
    static gboolean AboutEnterCB(GtkWidget *widget, GdkEventCrossing *crossing, gpointer data);
    static gboolean AboutLeaveCB(GtkWidget *widget, GdkEventCrossing *crossing, gpointer data);
    static gboolean AboutCB(GtkWidget *widget, GdkEvent *event, gpointer data)
    { ((CInstaller *)data)->ShowAbout(); return TRUE; }
    static void CancelCB(GtkWidget *widget, gpointer data);
    static void BackCB(GtkWidget *widget, gpointer data);
    static void NextCB(GtkWidget *widget, gpointer data);
    static gboolean DeleteCB(GtkWidget *widget, GdkEvent *event, gpointer data);
};

#endif
