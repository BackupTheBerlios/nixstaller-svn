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
#include "installscreen.h"
#include "main/lua/luaclass.h"
#include "tui/button.h"
#include "tui/box.h"
#include "tui/dialog.h"
#include "tui/separator.h"

// -------------------------------------
// Main installer screen
// -------------------------------------

CInstaller::CInstaller() : m_CurrentScreen(0)
{
    AddWidget(m_pScreenBox = new NNCurses::CBox(NNCurses::CBox::VERTICAL, false, 1));
    
    m_pButtonBox = new NNCurses::CBox(NNCurses::CBox::HORIZONTAL, false);
    m_pButtonBox->StartPack(m_pCancelButton = new NNCurses::CButton("Cancel"), false, false, 0, 1);
    m_pButtonBox->EndPack(m_pNextButton = new NNCurses::CButton("Next"), false, false, 0, 1);
    m_pButtonBox->EndPack(m_pPrevButton = new NNCurses::CButton("Back"), false, false, 0, 1);
    EndPack(m_pButtonBox, false, false, 0, 0);
    
    EndPack(new NNCurses::CSeparator(ACS_HLINE), false, false, 0, 0);
}

bool CInstaller::FirstValidScreen(NNCurses::CWidget *start)
{
    TScreenList::iterator it = std::find(m_InstallScreens.begin(), m_InstallScreens.end(), start);
    bool ret = (start == m_InstallScreens.front());
    
    if (!ret)
    {
        ret = true;
        do
        {
            it--;
            if ((*it)->CanActivate())
            {
                ret = false;
                break;
            }
        }
        while (it != m_InstallScreens.begin());
    }
    
    return ret;
}

bool CInstaller::LastValidScreen(NNCurses::CWidget *start)
{
    TScreenList::iterator it = std::find(m_InstallScreens.begin(), m_InstallScreens.end(), start);
    bool ret = (start == m_InstallScreens.back());
    
    if (!ret)
    {
        ret = true;
        it++;
        while (it != m_InstallScreens.end())
        {
            if ((*it)->CanActivate())
            {
                ret = false;
                break;
            }
            it++;
        }
    }
    
    return ret;
}

void CInstaller::UpdateButtons(void)
{
    if (FirstValidScreen(m_InstallScreens[m_CurrentScreen]) && !m_InstallScreens[m_CurrentScreen]->HasPrevWidgets())
        m_pPrevButton->Enable(false);
    else
        m_pPrevButton->Enable(true);
    
    if (LastValidScreen(m_InstallScreens[m_CurrentScreen]) && !m_InstallScreens[m_CurrentScreen]->HasNextWidgets())
        m_pNextButton->SetText(GetTranslation("Finish"));
    else
        m_pNextButton->SetText(GetTranslation("Next"));
}

void CInstaller::PrevScreen()
{
    if (m_InstallScreens[m_CurrentScreen]->SubBack())
    {
        UpdateButtons();
        
        if (CanFocusChilds(m_pScreenBox))
            FocusWidget(m_pScreenBox);
        
        return;
    }
    
    TScreenList::iterator it = m_InstallScreens.begin() + m_CurrentScreen;
    
    while (it != m_InstallScreens.begin())
    {
        it--;
        
        if ((*it)->CanActivate())
        {
            m_InstallScreens[m_CurrentScreen]->Enable(false);
            ActivateScreen(*it);
            UpdateButtons();
            break;
        }
    }
}

void CInstaller::NextScreen()
{
    if (m_InstallScreens[m_CurrentScreen]->SubNext())
    {
        UpdateButtons();
        
        if (CanFocusChilds(m_pScreenBox))
            FocusWidget(m_pScreenBox);

        return;
    }
    
    TScreenList::iterator it = m_InstallScreens.begin() + m_CurrentScreen;
    
    while (*it != m_InstallScreens.back())
    {
        it++;

        if ((*it)->CanActivate())
        {
            m_InstallScreens[m_CurrentScreen]->Enable(false);
            ActivateScreen(*it);
            UpdateButtons();
            return;
        }
    }
    
    // No screens left
    NNCurses::TUI.Quit();
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

void CInstaller::ActivateScreen(CInstallScreen *screen)
{
    screen->Enable(true);
    screen->Activate();
    
    m_CurrentScreen = 0;
    while (m_InstallScreens.at(m_CurrentScreen) != screen)
        m_CurrentScreen++;
    
    if (CanFocusChilds(m_pScreenBox))
        FocusWidget(m_pScreenBox);
    else
    {
        m_pButtonBox->FocusWidget(m_pNextButton);
        FocusWidget(m_pButtonBox);
    }
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
            FocusWidget(m_pButtonBox);
            m_pButtonBox->FocusWidget(m_pNextButton);
            return true;
        }
    }
    
    return false;
}

bool CInstaller::CoreHandleKey(chtype key)
{
    if (NNCurses::IsEscape(key))
    {
        AskQuit();
        return true;
    }
    
    return CNCursBase::CoreHandleKey(key);
}

void CInstaller::InitLua()
{
    CNCursBase::InitLua();
    CBaseInstall::InitLua();
}

void CInstaller::Init(int argc, char **argv)
{
    CBaseInstall::Init(argc, argv);
    
    UpdateLanguage();

    for (TScreenList::iterator it=m_InstallScreens.begin(); it!=m_InstallScreens.end(); it++)
    {
        if ((*it)->CanActivate())
        {
            ActivateScreen(*it);
            break;
        }
    }
}

void CInstaller::UpdateLanguage()
{
    UpdateButtons();
}

CBaseScreen *CInstaller::CreateScreen(const std::string &title)
{
    CInstallScreen *ret = new CInstallScreen(title);
    ret->Enable(false);
    m_pScreenBox->AddWidget(ret);
    return ret;
}

void CInstaller::AddScreen(int luaindex)
{
    CInstallScreen *screen = NLua::CheckClassData<CInstallScreen>("screen", luaindex);
    m_InstallScreens.push_back(screen);
}
