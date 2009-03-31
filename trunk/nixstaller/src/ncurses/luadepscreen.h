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

#ifndef NCURS_LUADEPSCREEN_H
#define NCURS_LUADEPSCREEN_H

#include "main/frontend/luadepscreen.h"
#include "tui/window.h"

namespace NNCurses {
class CButton;
class CTextField;
}

class CInstaller;

class CLuaDepScreen: public CBaseLuaDepScreen, public NNCurses::CWindow
{
    NNCurses::CButton *m_pCancelButton, *m_pRefreshButton, *m_pIgnoreButton;
    NNCurses::CTextField *m_pTextField;
    bool m_bClose;
    CInstaller *m_pInstaller;
    
    virtual void CoreUpdateList(void);
    virtual void CoreRun(void);
    virtual bool CoreHandleEvent(NNCurses::CWidget *emitter, int type);
    
public:
    CLuaDepScreen(CInstaller *inst, int f);
};


#endif
