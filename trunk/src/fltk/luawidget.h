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

#ifndef FLTK_LUAWIDGET_H
#define FLTK_LUAWIDGET_H

#include "main/install/luawidget.h"

class Fl_Box;
class Fl_Group;

class CLuaWidget: virtual public CBaseLuaWidget
{
    Fl_Group *m_pMainGroup;
    
    virtual void CoreSetTitle(void);
    virtual void CoreActivateWidget(void){}
    
protected:
    void LabelSize(int &w, int &h) const;
    
public:
    CLuaWidget(void);
    
    Fl_Group *GetGroup(void) { return m_pMainGroup; }
};

#endif
