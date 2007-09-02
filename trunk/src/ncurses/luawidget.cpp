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
#include "tui/tui.h"
#include "tui/label.h"

// -------------------------------------
// Base NCurses Lua Widget Class
// -------------------------------------

CLuaWidget::CLuaWidget() : NNCurses::CBox(NNCurses::CBox::VERTICAL, false), m_pTitle(NULL), m_iMaxWidth(0)
{
    StartPack(m_pTitleBox = new NNCurses::CBox(NNCurses::CBox::VERTICAL, false), false, false, 0, 0);
    m_pTitleBox->Enable(false);
}

void CLuaWidget::CoreSetTitle()
{
    if (!GetTitle().empty())
    {
        if (!m_pTitle)
        {
            m_pTitleBox->Enable(true);
            m_pTitleBox->AddWidget(m_pTitle = new NNCurses::CLabel(GetTranslation(GetTitle()), false));
            m_pTitle->SetMaxReqWidth(m_iMaxWidth);
        }
        else
            m_pTitle->SetText(GetTranslation(GetTitle()));
    }
}

void CLuaWidget::CoreActivateWidget()
{
    if (!Focused())
        SetNextFocWidget(false); // Set first valid widget
}

void CLuaWidget::SetMaxWidth(int maxw)
{
    m_iMaxWidth = maxw;
    if (m_pTitle)
        m_pTitle->SetMaxReqWidth(maxw);
}
