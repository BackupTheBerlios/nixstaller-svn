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

#ifndef GTK_LUACHECKBOX_H
#define GTK_LUACHECKBOX_H

#include <vector>
#include "main/frontend/luacheckbox.h"
#include "luawidget.h"


class CLuaCheckbox: public CBaseLuaCheckbox, public CLuaWidget
{
    std::vector<GtkWidget *> m_Checkboxes;
    bool m_bInit;
    
    virtual bool Enabled(TSTLVecSize n);
    virtual void Enable(TSTLVecSize n, bool b);
    virtual void AddOption(const std::string &label, TSTLVecSize n);
    virtual void DelOption(TSTLVecSize n);
    virtual void CoreUpdateLanguage(void);
    virtual void CoreActivateWidget(void);
    
public:
    CLuaCheckbox(const char *desc, const TOptions &l, const std::vector<TSTLVecSize> &e);
    
    static void ToggleCB(GtkToggleButton *togglebutton, gpointer data);
};

#endif
