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

#include "luawidget.h"
#include "tui/label.h"

// -------------------------------------
// Base Lua NCurses Widget Class
// -------------------------------------

CLuaWidget::CLuaWidget(const char *title) : NNCurses::CBox(NNCurses::CBox::VERTICAL, false), m_pTitle(NULL)
{
    if (title && *title)
    {
        m_Title = title;
        StartPack(m_pTitle = new NNCurses::CLabel(GetTranslation(m_Title)), false, false, 0, 0);
    }
}

void CLuaWidget::UpdateLanguage()
{
    if (m_pTitle)
        m_pTitle->SetText(GetTranslation(m_Title));
    CoreUpdateLanguage();
}
