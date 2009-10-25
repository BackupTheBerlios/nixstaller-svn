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

#include <algorithm>
#include <stdlib.h>

#include "main/frontend/utils.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "installer.h"
#include "installscreen.h"
#include "luadepscreen.h"
#include "luaprogressdialog.h"
#include "tui/button.h"
#include "tui/box.h"
#include "tui/dialog.h"
#include "tui/separator.h"
#include "tui/textfield.h"

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

bool CInstaller::FirstValidScreen()
{
    TScreenList::iterator it = std::find(m_InstallScreens.begin(), m_InstallScreens.end(),
                                         m_InstallScreens[m_CurrentScreen]);
    bool ret = (m_InstallScreens[m_CurrentScreen] == m_InstallScreens.front());
    
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

bool CInstaller::LastValidScreen()
{
    TScreenList::iterator it = std::find(m_InstallScreens.begin(), m_InstallScreens.end(),
                                         m_InstallScreens[m_CurrentScreen]);
    bool ret = (m_InstallScreens[m_CurrentScreen] == m_InstallScreens.back());
    
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
    if (m_InstallScreens.empty() || (FirstValidScreen() &&
         !m_InstallScreens[m_CurrentScreen]->HasPrevWidgets()))
        m_pPrevButton->Enable(false);
    else if (!m_bPrevButtonLocked)
        m_pPrevButton->Enable(true);
    
    if (m_InstallScreens.empty() || (LastValidScreen() &&
         !m_InstallScreens[m_CurrentScreen]->HasNextWidgets()))
        m_pNextButton->SetText(GetTranslation("Finish"));
    else
        m_pNextButton->SetText(GetTranslation("Next"));
}

void CInstaller::PrevScreen()
{
    if (m_InstallScreens[m_CurrentScreen]->SubBack())
    {
        UpdateButtons();
        return;
    }
    
    if (!m_InstallScreens[m_CurrentScreen]->Back())
        return;
    
    TScreenList::iterator it = m_InstallScreens.begin() + m_CurrentScreen;
    
    while (it != m_InstallScreens.begin())
    {
        it--;
        
        if ((*it)->CanActivate())
        {
            m_InstallScreens[m_CurrentScreen]->Enable(false);
            ActivateScreen(*it, true);
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
        return;
    }
    
    if (!m_InstallScreens[m_CurrentScreen]->Next())
        return;

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

void CInstaller::ActivateScreen(CInstallScreen *screen, bool sublast)
{
    screen->Enable(true);
    CBaseAttInstall::ActivateScreen(screen);
    if (sublast)
        screen->SubLast();
    
    m_CurrentScreen = 0;
    while (m_InstallScreens.at(m_CurrentScreen) != screen)
        m_CurrentScreen++;
    
    if (!CanFocusChilds(m_pScreenBox))
        m_pNextButton->ReqFocus();
}

char *CInstaller::GetPassword(const char *str)
{
    return StrDup(NNCurses::InputBox(str, "", 0, '*').c_str());
}

void CInstaller::ShowAbout()
{
    NNCurses::TColorPair fc(COLOR_GREEN, COLOR_BLUE), dfc(COLOR_WHITE, COLOR_BLUE);
    NNCurses::CDialog *dialog = NNCurses::CreateBaseDialog(fc, dfc, 25, 0);
    
    NNCurses::CTextField *text = new NNCurses::CTextField(35, 12, false);
    text->LoadFile(GetAboutFName());
    dialog->AddWidget(text);
    
    dialog->AddButton(new NNCurses::CButton(GetTranslation("OK")));
    
    NNCurses::TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    delete dialog;
}

void CInstaller::MsgBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    NNCurses::MessageBox(text);
    
    free(text);
}

bool CInstaller::YesNoBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    bool ret = NNCurses::YesNoBox(text);
    
    free(text);
    
    return ret;
}

int CInstaller::ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...)
{
    char *text;
    va_list v;
    
    va_start(v, button3);
    vasprintf(&text, str, v);
    va_end(v);
    
    NNCurses::TColorPair fc(COLOR_GREEN, COLOR_BLUE), dfc(COLOR_WHITE, COLOR_BLUE);
    NNCurses::CDialog *dialog = NNCurses::CreateBaseDialog(fc, dfc, 35, 0, text);
        
    NNCurses::CButton *buttons[3];
    
    dialog->AddButton(buttons[0] = new NNCurses::CButton(button1));
    dialog->AddButton(buttons[1] = new NNCurses::CButton(button2));
    
    if (button3)
        dialog->AddButton(buttons[2] = new NNCurses::CButton(button3));
    
    NNCurses::TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    int ret = -1;
    for (int i=0; i<3; i++)
    {
        if (dialog->ActivatedWidget() == buttons[i])
        {
            ret = i;
            break;
        }
    }
    
    delete dialog;
    free(text);
    
    return ret;
}

void CInstaller::WarnBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    NNCurses::WarningBox(text);
    
    free(text);
}

int CInstaller::TextWidth(const char *str)
{
    return static_cast<int>(MBWidth(str));
}

void CInstaller::CoreUpdate()
{
    CBaseAttInstall::CoreUpdate();
    NNCurses::TUI.Run(0);
}

void CInstaller::LockScreen(bool cancel, bool prev, bool next)
{
    m_pCancelButton->Enable(!cancel);
    m_pPrevButton->Enable(!prev);
    m_pNextButton->Enable(!next);
    m_bPrevButtonLocked = prev;
}

bool CInstaller::CoreHandleEvent(NNCurses::CWidget *emitter, int type)
{
    if (NNCurses::CWindow::CoreHandleEvent(emitter, type))
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
            if (AskQuit())
                throw Exceptions::CExUser();
            return true;
        }
        else if (m_pNextButton->Enabled())
        {
            // All unhandled callback events will focus next button.
            // This is usefull when user presses enter key in menus and such
            m_pNextButton->ReqFocus();
            return true;
        }
    }
    
    return false;
}

bool CInstaller::CoreHandleKey(wchar_t key)
{
    if (NNCurses::CWindow::CoreHandleKey(key))
        return true;
    
    if (key == KEY_F(2))
    {
        ShowAbout();
        return true;
    }
    else if (NNCurses::IsEscape(key))
    {
        if (m_pCancelButton->Enabled() && AskQuit())
            throw Exceptions::CExUser();
        return true;
    }
    
    return false;
}

void CInstaller::CoreGetButtonDescs(NNCurses::TButtonDescList &list)
{
    list.push_back(NNCurses::TButtonDescPair("F2", "About"));
    CWindow::CoreGetButtonDescs(list);
}

void CInstaller::InitLua()
{
    // Overide print function
    NLua::RegisterFunction(LuaLog, "print", NULL, this);
    CBaseAttInstall::InitLua();
}

void CInstaller::Init(int argc, char **argv)
{
    CBaseAttInstall::Init(argc, argv);
    
    UpdateButtons();

    for (TScreenList::iterator it=m_InstallScreens.begin(); it!=m_InstallScreens.end(); it++)
    {
        if ((*it)->CanActivate())
        {
            ActivateScreen(*it);
            break;
        }
    }
}

void CInstaller::CoreUpdateLanguage()
{
    CBaseAttInstall::CoreUpdateLanguage();
    m_pCancelButton->SetText(GetTranslation("Cancel"));
    m_pPrevButton->SetText(GetTranslation("Back"));
    UpdateButtons();
}

CBaseScreen *CInstaller::CreateScreen(const std::string &title)
{
    CInstallScreen *ret = new CInstallScreen(title, Preview());
    ret->Enable(false);
    m_pScreenBox->AddWidget(ret);
    return ret;
}

void CInstaller::CoreAddScreen(CBaseScreen *screen)
{
    m_InstallScreens.push_back(dynamic_cast<CInstallScreen *>(screen));
}

CBaseLuaProgressDialog *CInstaller::CoreCreateProgDialog(int r)
{
    CLuaProgressDialog *dialog = new CLuaProgressDialog(r);
    dialog->SetFColors(COLOR_GREEN, COLOR_BLUE);
    dialog->SetDFColors(COLOR_WHITE, COLOR_BLUE);
    dialog->SetMinWidth(50);
    NNCurses::TUI.AddGroup(dialog, true);
    return dialog;
}

CBaseLuaDepScreen *CInstaller::CoreCreateDepScreen(int f)
{
    CLuaDepScreen *dialog = new CLuaDepScreen(this, f);
    dialog->SetFColors(COLOR_GREEN, COLOR_BLUE);
    dialog->SetDFColors(COLOR_WHITE, COLOR_BLUE);
    dialog->SetMinWidth(50);
    NNCurses::TUI.AddGroup(dialog, true);
    return dialog;
}

void CInstaller::Run()
{
    while (NNCurses::TUI.Run())
        Update();
}
