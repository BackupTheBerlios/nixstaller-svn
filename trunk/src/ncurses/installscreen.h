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

#ifndef INSTALL_SCREEN_H
#define INSTALL_SCREEN_H

#include "main/install/basescreen.h"
#include "tui/tui.h"
#include "tui/box.h"

namespace NNCurses {
class CLabel;
}


class CInstallScreen: public CBaseScreen, public NNCurses::CBox
{
    NNCurses::CLabel *m_pTitle;
    
    // UNDONE
    virtual CBaseLuaInputField *CreateInputField(const char *label, const char *desc, const char *val,
            int max, const char *type) {}
    virtual CBaseLuaCheckbox *CreateCheckbox(const char *desc, const std::vector<std::string> &l) {}
    virtual CBaseLuaRadioButton *CreateRadioButton(const char *desc, const std::vector<std::string> &l) {}
    virtual CBaseLuaDirSelector *CreateDirSelector(const char *desc, const char *val) {}
    virtual CBaseLuaCFGMenu *CreateCFGMenu(const char *desc) {}
    virtual void CoreUpdateLanguage(void);
    
public:
    CInstallScreen(const std::string &title);
};

#endif
