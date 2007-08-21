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

#ifndef GTK_LUACFGMENU_H
#define GTK_LUACFGMENU_H

#include "main/install/luacfgmenu.h"
#include "luawidget.h"

class CLuaCFGMenu: public CBaseLuaCFGMenu, public CLuaWidget
{
    GtkWidget *m_pVarListView, *m_pInputField, *m_pComboBox, *m_pDirInput, *m_pDirButtonLabel, *m_pDirInputBox;
    bool m_bInitSelection;
    
    enum { COLUMN_TITLE, COLUMN_DESC, COLUMN_VAR, COLUMN_N };
    
    virtual void CoreAddVar(const char *name);
    virtual void CoreUpdateLanguage(void);
    virtual void CoreActivateWidget(void);
    
    GtkWidget *CreateVarListBox(void);
    GtkWidget *CreateInputField(void);
    GtkWidget *CreateComboBox(void);
    GtkWidget *CreateDirSelector(void);
    
    std::string CurSelection(void);
    
    void UpdateSelection(const gchar *var);
    void SetComboBox(const std::string &var);
    
public:
    CLuaCFGMenu(const char *desc);
    
    static void SelectionCB(GtkTreeSelection *selection, gpointer data);
    static void InputChangedCB(GtkEditable *widget, gpointer data);
    static void ComboBoxChangedCB(GtkComboBox *widget, gpointer data);
    static void BrowseCB(GtkWidget *widget, gpointer data);
};

#endif
