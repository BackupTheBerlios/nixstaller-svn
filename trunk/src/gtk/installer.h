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
    bool m_bInstallFiles;
    GtkWidget *m_pCancelButton, *m_pPrevButton, *m_pNextButton;

protected:
    virtual void ChangeStatusText(const char *str) {};
    virtual void AddInstOutput(const std::string &str) {};
    virtual void SetProgress(int percent) {};
    virtual void InstallThink(void) { };

public:

    CInstaller(void) : m_bInstallFiles(false) { };
    virtual ~CInstaller(void) {};

//     virtual void InitLua(void) {};
    virtual void Init(int argc, char **argv);
    virtual void Install(void) {};
    virtual void UpdateLanguage(void) {};
    virtual CBaseCFGScreen *CreateCFGScreen(const char *title){};
    
    static void AboutCB(GtkWidget *widget, gpointer data) { ((CInstaller *)data)->ShowAbout(); }
};

#endif
