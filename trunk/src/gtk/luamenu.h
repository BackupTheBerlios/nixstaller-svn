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

#ifndef NCURSES_LUAMENU_H
#define NCURSES_LUAMENU_H

#include "main/install/luamenu.h"
#include "luawidget.h"

class CLuaMenu: public CBaseLuaMenu, public CLuaWidget
{
    enum { COLUMN_TITLE, COLUMN_VAR, COLUMN_N };
    
    GtkWidget *m_pMenu;
    bool m_bInitSel;
    
    virtual const char *Selection(void);
    virtual void Select(TSTLVecSize n);
    virtual void CoreUpdateLanguage(void);
    virtual void CoreActivateWidget(void);
    
    GtkWidget *CreateMenu(void);
    
public:
    CLuaMenu(const char *desc, const TOptions &l);
    
    static void SelectionCB(GtkTreeSelection *selection, gpointer data);
};

#endif
