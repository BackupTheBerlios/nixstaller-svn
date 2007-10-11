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

#ifndef FLTK_LUAINPUT_H
#define FLTK_LUAINPUT_H

#include "main/install/luainput.h"
#include "luawidget.h"

class Fl_Box;
class Fl_Input;
class Fl_Pack;
class Fl_Widget;

class CLuaInputField: public CBaseLuaInputField, public CLuaWidget
{
    Fl_Pack *m_pPack;
    Fl_Box *m_pLabel;
    Fl_Input *m_pInputField;
    std::string m_Text;
    
    virtual void CoreSetValue(const char *v);
    virtual const char *CoreGetValue(void);
    virtual void CoreUpdateLabelWidth(void) { UpdateSize(); }
    virtual void CoreUpdateLanguage(void);
    virtual int CoreRequestWidth(void);
    virtual int CoreRequestHeight(int maxw);
    virtual void UpdateSize(void);
    
    int PackSpacing(void) const { return 5; }

public:
    CLuaInputField(const char *label, const char *desc, const char *val, int max, const char *type);
    
    static void InputChangedCB(Fl_Widget *w, void *p);
};

#endif
