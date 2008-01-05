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

#ifndef DIALOG
#define DIALOG

#include "window.h"

namespace NNCurses {

class CButton;

class CDialog: public CWindow
{
    CBox *m_pButtonBox;
    CWidget *m_pActivatedWidget;
    
protected:
    virtual bool CoreHandleEvent(CWidget *emitter, int event);
    virtual void CoreGetButtonDescs(TButtonDescList &list);
    virtual bool CoreRun(void);
    
public:
    CDialog(void);
    
    void AddButton(CButton *button, bool expand=true, bool start=true);
    bool Run(void) { return CoreRun(); }
    CWidget *ActivatedWidget(void) { return m_pActivatedWidget; }
};

// Utils
CDialog *CreateBaseDialog(TColorPair fc, TColorPair dfc, int minw=0, int minh=0, const std::string &text="");
void MessageBox(const std::string &msg);
void WarningBox(const std::string &msg);
bool YesNoBox(const std::string &msg);
std::string InputBox(const std::string &msg, const std::string &init="", int max=0, chtype out=0);
std::string DirectoryBox(const std::string &msg, const std::string &start);

}

#endif
