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

#ifndef INSTALLER_H
#define INSTALLER_H

class CInstaller: public CGTKBase, public CBaseInstall
{
    GtkWidget *m_pCancelLabel, *m_pBackLabel, *m_pNextLabel;
    GtkWidget *m_pWizard;
    
    void InitAboutSection(GtkWidget *parentbox);
    void InitScreenSection(GtkWidget *parentbox);
    void InitButtonSection(GtkWidget *parentbox);
    
    void Back(void) { gtk_notebook_prev_page(GTK_NOTEBOOK(m_pWizard)); };
    void Next(void) { gtk_notebook_next_page(GTK_NOTEBOOK(m_pWizard)); };

    bool AskQuit(void);
    
    virtual CBaseScreen *CreateScreen(const std::string &title);
    virtual void AddScreen(int luaindex);
    virtual void InstallThink(void) {}
    virtual void LockScreen(bool cancel, bool prev, bool next) {}

public:
    virtual void Init(int argc, char **argv);
    virtual void UpdateLanguage(void) {};
    
    static void AboutCB(GtkWidget *widget, gpointer data) { ((CInstaller *)data)->ShowAbout(); }
    static void CancelCB(GtkWidget *widget, gpointer data);
    static void BackCB(GtkWidget *widget, gpointer data) { ((CInstaller *)data)->Back(); }
    static void NextCB(GtkWidget *widget, gpointer data) { ((CInstaller *)data)->Next(); }
    static gboolean DeleteCB(GtkWidget *widget, GdkEvent *event, gpointer data);
};

#endif
