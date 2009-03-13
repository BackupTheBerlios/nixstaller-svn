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

#ifndef GTK_LUADEPSCREEN_H
#define GTK_LUADEPSCREEN_H

#include "main/install/luadepscreen.h"

class CInstaller;

class CLuaDepScreen: public CBaseLuaDepScreen
{
    enum { COLUMN_NAME, COLUMN_DESC, COLUMN_PROB, COLUMN_N };
    
    GtkWidget *m_pDialog, *m_pListView;
    CInstaller *m_pInstaller;
    
    bool m_bClose;
    
    virtual void CoreUpdateList(void);
    virtual void CoreRun(void);
    
    GtkWidget *CreateListBox(void);
    
public:
    CLuaDepScreen(GtkWidget *parent, CInstaller *inst, int f);
    virtual ~CLuaDepScreen(void);
    
    static gboolean DeleteCB(GtkWidget *widget, GdkEvent *event, gpointer data);
    static void CancelCB(GtkWidget *widget, gpointer data);
    static void RefreshCB(GtkWidget *widget, gpointer data);
    static void IgnoreCB(GtkWidget *widget, gpointer data);
};


#endif
