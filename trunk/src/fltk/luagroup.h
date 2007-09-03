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

#ifndef FLTK_LUAGROUP_H
#define FLTK_LUAGROUP_H

#include "main/install/luagroup.h"

class CLuaWidget;
class Fl_Group;
class Fl_Pack;

class CLuaGroup: public CBaseLuaGroup
{
    Fl_Pack *m_pMainPack;

    virtual CBaseLuaInputField *CreateInputField(const char *label, const char *desc, const char *val,
            int max, const char *type){}
    virtual CBaseLuaCheckbox *CreateCheckbox(const char *desc, const std::vector<std::string> &l);
    virtual CBaseLuaRadioButton *CreateRadioButton(const char *desc, const std::vector<std::string> &l){}
    virtual CBaseLuaDirSelector *CreateDirSelector(const char *desc, const char *val);
    virtual CBaseLuaCFGMenu *CreateCFGMenu(const char *desc);
    virtual CBaseLuaMenu *CreateMenu(const char *desc, const std::vector<std::string> &l){}
    virtual CBaseLuaImage *CreateImage(const char *file){}
    virtual CBaseLuaProgressBar *CreateProgressBar(const char *desc){}
    virtual CBaseLuaTextField *CreateTextField(const char *desc, bool wrap){}
    virtual CBaseLuaLabel *CreateLabel(const char *title);
    
    int WidgetSpacing(void) const { return 10; }
   
    void AddWidget(CLuaWidget *w);
    
public:
    CLuaGroup(void);
    virtual ~CLuaGroup(void) { DeleteWidgets(); }
    
    Fl_Group *GetGroup(void);
    void SetSize(int maxw, int maxh);
};

#endif
