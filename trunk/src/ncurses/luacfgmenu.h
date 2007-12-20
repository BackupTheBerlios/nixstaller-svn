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

#ifndef NCURSES_LUACFGMENU_H
#define NCURSES_LUACFGMENU_H

#include "main/install/luacfgmenu.h"
#include "luawidget.h"

namespace NNCurses {
    class CMenu;
    class CTextField;
}

class CLuaCFGMenu: public CBaseLuaCFGMenu, public CLuaWidget
{
    NNCurses::CMenu *m_pMenu;
    NNCurses::CTextField *m_pDescWin;
    
    virtual void CoreAddVar(const char *name);
    virtual void CoreUpdateVar(const char *name);
    virtual void CoreUpdateLanguage(void);
    
    SEntry *GetCurEntry(void);
    void UpdateDesc(void);
    void ShowChoiceMenu(void);
    
protected:
    virtual void CoreInit(void) { UpdateDesc(); }
    virtual bool CoreHandleEvent(NNCurses::CWidget *emitter, int event);
    virtual void CoreGetButtonDescs(NNCurses::TButtonDescList &list);
    
public:
    CLuaCFGMenu(const char *desc);
};

#endif
