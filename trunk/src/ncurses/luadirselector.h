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

#ifndef NCURSES_LUADIRSELECTOR_H
#define NCURSES_LUADIRSELECTOR_H

#include "main/install/luadirselector.h"
#include "luawidget.h"

namespace NNCurses {
    class CButton;
    class CInputField;
}


class CLuaDirSelector: public CBaseLuaDirSelector, public CLuaWidget
{
    NNCurses::CInputField *m_pDirInput;
    NNCurses::CButton *m_pBrowseButton;
    
    virtual const char *GetDir(void){}
    virtual void SetDir(const char *dir){}
    virtual void CoreUpdateLanguage(void);
    
protected:
    virtual bool CoreHandleEvent(NNCurses::CWidget *emitter, int event);
    
public:
    CLuaDirSelector(const char *desc, const char *val);
};

#endif
