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

#include "installer.h"
#include "luadepscreen.h"
#include "tui/box.h"
#include "tui/button.h"
#include "tui/label.h"
#include "tui/separator.h"
#include "tui/textfield.h"
#include "tui/tui.h"

// -------------------------------------
// NCurses Lua Dependency Screen Class
// -------------------------------------

CLuaDepScreen::CLuaDepScreen(CInstaller *inst, int f) : CBaseLuaDepScreen(f), m_bClose(false),
                                                        m_pInstaller(inst)
{
    StartPack(new NNCurses::CLabel(GetTitle()), false, false, 0, 0);
    AddWidget(m_pTextField = new NNCurses::CTextField(20, 10, false));
    StartPack(new NNCurses::CSeparator(ACS_HLINE), true, true, 0, 0);
    
    NNCurses::CBox *bbox = new NNCurses::CBox(NNCurses::CBox::HORIZONTAL, true);
    StartPack(bbox, false, false, 0, 0);
    bbox->AddWidget(m_pCancelButton = new NNCurses::CButton(GetTranslation("Cancel")));
    bbox->AddWidget(m_pRefreshButton = new NNCurses::CButton(GetTranslation("Refresh")));
    bbox->AddWidget(m_pIgnoreButton = new NNCurses::CButton(GetTranslation("Ignore")));
    
    Enable(false);
}

void CLuaDepScreen::CoreUpdateList()
{
    if (!Enabled())
    {
        Enable(true);
        NNCurses::TUI.ActivateGroup(this);
    }
        
    m_pTextField->ClearText();
    
    for (TDepList::const_iterator it=GetDepList().begin(); it!=GetDepList().end(); it++)
    {
        m_pTextField->AddText(CreateText("%s: %s\n", GetTranslation("Name"),
                              GetTranslation(it->name).c_str()));
        m_pTextField->AddText(CreateText("%s: %s\n", GetTranslation("Description"),
                              GetTranslation(it->description).c_str()));
        m_pTextField->AddText(CreateText("%s: %s\n\n", GetTranslation("Problem"),
                              GetTranslation(it->problem).c_str()));
    }
    
    if (GetDepList().empty())
        m_bClose = true;
}

void CLuaDepScreen::CoreRun()
{
    while (!m_bClose)
    {
        if (!NNCurses::TUI.Run(1000))
            m_pInstaller->AskQuit();
    }
}

bool CLuaDepScreen::CoreHandleEvent(NNCurses::CWidget *emitter, int type)
{
    if (NNCurses::CWindow::CoreHandleEvent(emitter, type))
        return true;
    
    if (type == EVENT_CALLBACK) 
    {
        if (emitter == m_pCancelButton)
        {
            m_pInstaller->AskQuit();
            return true;
        }
        else if (emitter == m_pRefreshButton)
        {
            Refresh();
            return true;
        }
        else if (emitter == m_pIgnoreButton)
        {
            m_bClose = true;
            return true;
        }
    }
    
    return false;
}
