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

#ifndef FLTK_LUAPROGRESSDIALOG_H
#define FLTK_LUAPROGRESSDIALOG_H

#include "main/install/luaprogressdialog.h"

class Fl_Box;
class Fl_Button;
class Fl_Progress;
class Fl_Widget;
class Fl_Window;

class CLuaProgressDialog: public CBaseLuaProgressDialog
{
    Fl_Window *m_pDialog;
    Fl_Box *m_pTitle, *m_pSecTitle;
    Fl_Progress *m_pProgBar, *m_pSecProgBar;
    Fl_Button *m_pCancelButton;
    
    virtual void CoreSetTitle(const char *title);
    virtual void CoreSetProgress(int progress);
    virtual void CoreEnableSecProgBar(bool enable);
    virtual void CoreSetSecTitle(const char *title);
    virtual void CoreSetSecProgress(int progress);
    virtual void CoreSetCancelButton(bool e);
    
    int WidgetHeight(void) const { return 20; }
    void UpdateWidgets(void);
    
public:
    CLuaProgressDialog(int r);
    virtual ~CLuaProgressDialog(void);
    
    static void CancelCB(Fl_Widget *w, void *p);
};

#endif

