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

#ifndef NCURSES_LUAGROUP_H
#define NCURSES_LUAGROUP_H

#include "tui/box.h"
#include "main/install/luagroup.h"

class CLuaGroup: public CBaseLuaGroup, public NNCurses::CBox
{
    bool m_bInitEnable;
    
    virtual CBaseLuaInputField *CreateInputField(const char *label, const char *desc, const char *val,
            int max, const char *type);
    virtual CBaseLuaCheckbox *CreateCheckbox(const char *desc, const std::vector<std::string> &l);
    virtual CBaseLuaRadioButton *CreateRadioButton(const char *desc, const std::vector<std::string> &l);
    virtual CBaseLuaDirSelector *CreateDirSelector(const char *desc, const char *val);
    virtual CBaseLuaCFGMenu *CreateCFGMenu(const char *desc);
    virtual CBaseLuaMenu *CreateMenu(const char *desc, const std::vector<std::string> &l);
    virtual CBaseLuaImage *CreateImage(const char *desc, const char *file);
    virtual void CoreUpdateLanguage(void);
    
protected:
    virtual void CoreAddWidget(NNCurses::CWidget *w);
    
public:
    CLuaGroup(void) : NNCurses::CBox(NNCurses::CBox::HORIZONTAL, false, 1), m_bInitEnable(true) { Enable(false); }
};

#endif
