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

#ifndef GTK_LUAINPUT_H
#define GTK_LUAINPUT_H

#include "main/install/luainput.h"
#include "luawidget.h"

class CLuaInputField: public CBaseLuaInputField, public CLuaWidget
{
    GtkWidget *m_pLabel, *m_pEntry;
    
    virtual void CoreSetValue(const char *v);
    virtual const char *CoreGetValue(void);
    virtual void CoreUpdateLabelWidth(void);
    virtual void CoreUpdateLanguage(void);
    virtual void CoreActivateWidget(void);
    
    void SetLabel(void);

public:
    CLuaInputField(const char *label, const char *desc, const char *val, int max, const char *type);
    
    static void InsertCB(GtkEditable *editable, gchar *nt, gint new_text_length, gint *position,
                         gpointer data);
    static void InputChangedCB(GtkEditable *widget, gpointer data);
};

#endif
