/*
    Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)

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
#include "tui/dialog.h"
#include "tui/button.h"
#include "tui/box.h"

// -------------------------------------
// Main installer screen
// -------------------------------------

CInstaller::CInstaller() : m_CurrentScreen(0)
{
    AddWidget(m_pScreenBox = new NNCurses::CBox(NNCurses::CBox::VERTICAL, false, 1));
    
    NNCurses::CBox *buttonbox = new NNCurses::CBox(NNCurses::CBox::HORIZONTAL, false);
    
    buttonbox->StartPack(m_pCancelButton = new NNCurses::CButton("Cancel"), false, false, 0, 1);
    buttonbox->EndPack(m_pNextButton = new NNCurses::CButton("Next"), false, false, 0, 1);
    buttonbox->EndPack(m_pPrevButton = new NNCurses::CButton("Previous"), false, false, 0, 1);
    
    EndPack(buttonbox, false, false, 0, 0);
}

void CInstaller::AskQuit()
{
    char *msg;
    if (m_bInstalling)
        msg = GetTranslation("Install commands are still running\n"
                "If you abort now this may lead to a broken installation\n"
                "Are you sure?");
    else
        msg = GetTranslation("This will abort the installation\nAre you sure?");
    
    if (NNCurses::YesNoBox(msg))
        throw Exceptions::CExUser();
}

void CInstaller::InstallThink()
{
    NNCurses::TUI.Run(0);
}

bool CInstaller::CoreHandleEvent(NNCurses::CWidget *emitter, int type)
{
    if (CNCursBase::CoreHandleEvent(emitter, type))
        return true;
    
    if (type == EVENT_CALLBACK)
    {
        if (emitter == m_pNextButton)
        {
            NextScreen();
            return true;
        }
        else if (emitter == m_pPrevButton)
        {
            PrevScreen();
            return true;
        }
        else if (emitter == m_pCancelButton)
        {
            AskQuit();
            return true;
        }
        else
        {
            // All unhandled callback events will focus next button.
            // This is usefull when user presses enter key in menus and such
            FocusWidget(m_pNextButton);
            return true;
        }
    }
    
    return false;
}

bool CInstaller::CoreHandleKey(NNCurses::chtype key)
{
    if (CNCursBase::CoreHandleKey(key))
        return true;
    
    if (NNCurses::IsEscape(key))
    {
        AskQuit();
        return true;
    }
    
    return false;
}

void CInstaller::InitLua()
{
    CNCursBase::InitLua();
    CBaseInstall::InitLua();
}

void CInstaller::Init(int argc, char **argv)
{
    CBaseInstall::Init(argc, argv);
}
