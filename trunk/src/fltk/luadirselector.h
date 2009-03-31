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

#ifndef FLTK_LUADIRSELECTOR_H
#define FLTK_LUADIRSELECTOR_H

#include "main/frontend/luadirselector.h"
#include "luawidget.h"

class Fl_Button;
class Fl_File_Input;
class Fl_Widget;

class CLuaDirSelector: public CBaseLuaDirSelector, public CLuaWidget
{
    Fl_File_Input *m_pDirInput;
    Fl_Button *m_pBrowseButton;
    
    virtual const char *GetDir(void);
    virtual void SetDir(const char *dir);
    virtual void CoreUpdateLanguage(void);
    virtual int CoreRequestWidth(void) { return 150; }
    virtual int CoreRequestHeight(int maxw);
    virtual void UpdateSize(void) { UpdateLayout(); }
    
    int Spacing(void) const { return 10; }
    void UpdateLayout(void);
    
public:
    CLuaDirSelector(const char *desc, const char *val);
    
    static void InputChangedCB(Fl_Widget *w, void *p);
    static void BrowseCB(Fl_Widget *w, void *p);
};

#endif
