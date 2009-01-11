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

#include "luatextfield.h"
#include "tui/textfield.h"

// -------------------------------------
// Lua Text Field Class
// -------------------------------------

CLuaTextField::CLuaTextField(const char *desc, bool wrap, const char *size) : CBaseLuaWidget(desc), m_iDefWidth(15)
{
    int height;
    if (!strcmp(size, "small"))
        height = 4;
    else if (!strcmp(size, "medium"))
        height = 6;
    else
        height = 8;
    AddWidget(m_pTextField = new NNCurses::CTextField(m_iDefWidth, height, wrap));
    m_pTextField->SetMaxLength(MaxSize(), ClearSize());
}

void CLuaTextField::Load(const char *file)
{
    m_pTextField->LoadFile(file);
}

void CLuaTextField::AddText(const char *text)
{
    m_pTextField->AddText(text);
}

void CLuaTextField::ClearText()
{
    m_pTextField->ClearText();
}

void CLuaTextField::UpdateFollow()
{
    m_pTextField->SetFollow(Follow());
}
