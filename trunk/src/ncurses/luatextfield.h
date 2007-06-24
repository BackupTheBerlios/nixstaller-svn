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

#ifndef NCURSES_LUATEXTFIELD_H
#define NCURSES_LUATEXTFIELD_H

#include "main/install/luatextfield.h"
#include "luawidget.h"

namespace NNCurses {
    class CTextField;
}

class CLuaTextField: public CBaseLuaTextField, public CLuaWidget
{
    NNCurses::CTextField *m_pTextField;
    const int m_iDefWidth, m_iDefHeight;
    
    virtual void Load(const char *file);
    virtual void AddText(const char *text);
    
public:
    CLuaTextField(const char *desc, bool wrap);
};

#endif
