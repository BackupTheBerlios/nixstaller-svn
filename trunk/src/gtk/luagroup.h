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

#ifndef GTK_LUAGROUP_H
#define GTK_LUAGROUP_H

#include "main/install/luagroup.h"
#include "luawidget.h"

class CInstallScreen;

class CLuaGroup: public CBaseLuaGroup
{
    GtkWidget *m_pBox;
    CInstallScreen *m_pInstallScreen;

    virtual CBaseLuaInputField *CreateInputField(const char *label, const char *desc, const char *val,
            int max, const char *type);
    virtual CBaseLuaCheckbox *CreateCheckbox(const char *desc, const std::vector<std::string> &l,
                                             const std::vector<TSTLVecSize> &e);
    virtual CBaseLuaRadioButton *CreateRadioButton(const char *desc, const std::vector<std::string> &l,
                                                   TSTLVecSize e);
    virtual CBaseLuaDirSelector *CreateDirSelector(const char *desc, const char *val);
    virtual CBaseLuaCFGMenu *CreateCFGMenu(const char *desc);
    virtual CBaseLuaMenu *CreateMenu(const char *desc, const std::vector<std::string> &l,
                                     TSTLVecSize e);
    virtual CBaseLuaImage *CreateImage(const char *file);
    virtual CBaseLuaProgressBar *CreateProgressBar(const char *desc);
    virtual CBaseLuaTextField *CreateTextField(const char *desc, bool wrap);
    virtual CBaseLuaLabel *CreateLabel(const char *title);
    
    int MaxWidgetWidth(void) const { return 600; }
    void AddWidget(CLuaWidget *w);
    
public:
    CLuaGroup(CInstallScreen *screen);
    virtual ~CLuaGroup(void) { DeleteWidgets(); }
    
    GtkWidget *GetBox(void) { return m_pBox; }
};

#endif
