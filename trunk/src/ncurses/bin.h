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

#ifndef BIN_H
#define BIN_H

#include "group.h"

namespace NNCurses {

class CBin: public CGroup
{
    int GetIdealW(CWidget *w) { return std::max(w->RequestWidth(), GetMinWidth()); }
    int GetIdealH(CWidget *w) { return std::max(w->RequestHeight(), GetMinHeight()); }
    void UpdateMySize(CWidget *w);
    void UpdateWidgetSize(CWidget *w);
    CWidget *GetChild(void) { return GetChildList().front(); }
    
protected:
    virtual bool HandleEvent(CWidget *emitter, int type);
    virtual void CoreAddWidget(CWidget *w);
    virtual int CoreRequestWidth(void);
    virtual int CoreRequestHeight(void);
};


}

#endif
