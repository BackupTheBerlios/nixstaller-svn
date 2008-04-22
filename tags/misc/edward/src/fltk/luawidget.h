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

#ifndef FLTK_LUAWIDGET_H
#define FLTK_LUAWIDGET_H

#include "main/install/luawidget.h"

class Fl_Box;
class Fl_Group;
class Fl_Pack;
class CLuaGroup;

class CLuaWidget: virtual public CBaseLuaWidget
{
    CLuaGroup *m_pParent;
    Fl_Pack *m_pMainPack;
    Fl_Box *m_pTitle;
    
    virtual void CoreSetTitle(void);
    virtual void CoreActivateWidget(void);
    virtual bool CoreExpand(void) { return true; }
    virtual int CoreRequestWidth(void) = 0;
    virtual int CoreRequestHeight(int maxw) = 0;
    virtual void UpdateSize(void) {}
    
protected:
    CLuaGroup *GetParent(void) { return m_pParent; }
    
public:
    CLuaWidget(void);
    
    void SetParent(CLuaGroup *p) { m_pParent = p; }
    Fl_Group *GetGroup(void);
    bool Expand(void) { return CoreExpand(); }
    int RequestWidth(void);
    int RequestHeight(int maxw);
    void SetSize(int w, int h);
};

#endif
