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

#ifndef NCURSES_LUACHECKBOX_H
#define NCURSES_LUACHECKBOX_H

#include "main/install/luacheckbox.h"
#include "luawidget.h"

namespace NNCurses {
    class CCheckbox;
}

class CLuaCheckbox: public CBaseLuaCheckbox, public CLuaWidget
{
    NNCurses::CCheckbox *m_pCheckbox;
    
    virtual bool Enabled(TSTLVecSize n);
    virtual void Enable(TSTLVecSize n, bool b);
    virtual void AddOption(const std::string &label, TSTLVecSize n);
    virtual void DelOption(TSTLVecSize n);
    virtual void CoreUpdateLanguage(void);
    
protected:
    virtual bool CoreHandleEvent(NNCurses::CWidget *emitter, int event);
    
public:
    CLuaCheckbox(const char *desc, const TOptions &l, const std::vector<TSTLVecSize> &e);
};

#endif
