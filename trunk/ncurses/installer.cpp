/*
    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.

    Nixstaller is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Nixstaller is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nixstaller; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
    conditions of the GNU General Public License cover the whole combination.

    In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
    software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
    DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
    such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
    you include the source code of that other code when and as the GNU GPL requires distribution of source code.

    Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
    versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
    version without this exception; this exception also makes it possible to release a modified version which carries forward
    this exception.
*/

#include "ncurses.h"

// -------------------------------------
// Main installer screen
// -------------------------------------

void CInstaller::Prev()
{
    if (m_CurrentScreenIt == m_InstallScreens.begin())
        return;
    
    if (!(*m_CurrentScreenIt)->Prev())
        return;
    
    std::list<CBaseScreen *>::iterator it = m_CurrentScreenIt;
    
    while (it != m_InstallScreens.begin())
    {
        it--;
        
        if ((*it)->Activate())
        {
            (*m_CurrentScreenIt)->Enable(false);
            (*it)->Enable(true);
            ActivateChild(*it); // Give screen focus
            m_CurrentScreenIt = it;
            break;
        }
    }
}

void CInstaller::Next()
{
    if ((*m_CurrentScreenIt != m_InstallScreens.back()) && (!(*m_CurrentScreenIt)->Next()))
        return;
    
    std::list<CBaseScreen *>::iterator it = m_CurrentScreenIt;
    
    while (*it != m_InstallScreens.back())
    {
        it++;
        
        if ((*it)->Activate())
        {
            (*m_CurrentScreenIt)->Enable(false);
            (*it)->Enable(true);
            ActivateChild(*it); // Give screen focus
            m_CurrentScreenIt = it;
            return;
        }
    }
    
    EndProg();
}

bool CInstaller::HandleKey(chtype ch)
{
    if (CNCursBase::HandleKey(ch))
        return true;
    
    if (ch == ESCAPE)
    {
        m_pCancelButton->Push();
        return true;
    }
    
    return false;
}

bool CInstaller::HandleEvent(CWidgetHandler *p, int type)
{
    if (type == EVENT_CALLBACK)
    {
        if (p == m_pNextButton)
        {
            Next();
            return true;
        }
        else if (p == m_pPrevButton)
        {
            Prev();
            return true;
        }
        else if (p == m_pCancelButton)
        {
            char *msg;
            if (m_bInstallFiles)
                msg = /*pInst->GetTranslation UNDONE*/("Install commands are still running\n"
                        "If you abort now this may lead to a broken installation\n"
                        "Are you sure?");
            else
                msg = /*pInst->GetTranslation UNDONE*/("This will abort the installation\nAre you sure?");
    
            if (YesNoBox(msg))
                EndProg();
            return true;
        }
        else if (p == *m_CurrentScreenIt)
        {
            ActivateChild(m_pNextButton);
            return true;
        }
    }
    
    return false;
}

bool CInstaller::InitLua()
{
    const int x=2, y=2, w=width()-4, h=m_pCancelButton->rely()-3;
    
    m_LuaVM.InitClass("welcomescreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(new CWelcomeScreen(this, h, w, y, x), "welcomescreen", "WelcomeScreen");
    
    m_LuaVM.InitClass("licensescreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(new CLicenseScreen(this, h, w, y, x), "licensescreen", "LicenseScreen");

    m_LuaVM.InitClass("selectdirscreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(new CSelectDirScreen(this, h, w, y, x), "selectdirscreen", "SelectDirScreen");

/*    m_LuaVM.InitClass("installscreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(new CLangScreen(this, h, w, y, x), "installscreen", "InstallScreen");*/
    
/*    m_LuaVM.InitClass("finishscreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(new CLangScreen(this, h, w, y, x), "finishscreen", "FinishScreen");*/
    
    if (!CBaseInstall::InitLua())
        return false;
}

bool CInstaller::Init(int argc, char **argv)
{
    SetTitle("Nixstaller");
    
    // Button width; longest text + 4 chars for focusing
    int bw = strlen("Cancel") + 4;

    m_pCancelButton = new CButton(this, 1, bw, height()-2, 2, "Cancel", 'r');
    m_pPrevButton = new CButton(this, 1, bw, height()-2, width()-(2*(bw+2)), "Back", 'r');
    m_pNextButton = new CButton(this, 1, bw, height()-2, width()-(bw+2), "Next", 'r');
    
    if (!CBaseInstall::Init(argc, argv))
        return false;
    
    const int x=2, y=2, w=width()-4, h=m_pCancelButton->rely()-3;
    m_InstallScreens.push_back(new CLangScreen(this, h, w, y, x));

    unsigned count = m_LuaVM.OpenArray("ScreenList");
    debugline("Count : %d\n", count);
    if (!count)
    {
        m_InstallScreens.push_back(new CWelcomeScreen(this, h, w, y, x));
        m_InstallScreens.push_back(new CLicenseScreen(this, h, w, y, x));
        m_InstallScreens.push_back(new CSelectDirScreen(this, h, w, y, x));
    }
    else
    {
        for (unsigned u=1; u<=count; u++)
        {
            CBaseScreen *screen = NULL;
            CBaseCFGScreen *cfgscreen = NULL;
            if (m_LuaVM.GetArrayClass<CBaseScreen *>(u, &screen, "welcomescreen") ||
                m_LuaVM.GetArrayClass<CBaseScreen *>(u, &screen, "licensescreen") ||
                m_LuaVM.GetArrayClass<CBaseScreen *>(u, &screen, "selectdirscreen") ||
                m_LuaVM.GetArrayClass<CBaseScreen *>(u, &screen, "installscreen") ||
                m_LuaVM.GetArrayClass<CBaseScreen *>(u, &screen, "finishscreen") ||
                m_LuaVM.GetArrayClass<CBaseCFGScreen *>(u, &cfgscreen, "cfgscreen"))
            {
                if (screen)
                    m_InstallScreens.push_back(screen);
                else if (cfgscreen)
                    m_InstallScreens.push_back((CCFGScreen *)cfgscreen);
            }
            else
                ThrowError(false, "Wrong type found in ScreenList variabale");
        }
        m_LuaVM.CloseArray();
    }
    
    bool initscreen = true;
    for (std::list<CBaseScreen *>::iterator it=m_InstallScreens.begin(); it!=m_InstallScreens.end(); it++)
    {
        if (initscreen && (*it)->Activate())
        {
            ActivateChild(*it); // Give screen focus
            m_CurrentScreenIt = it;
            initscreen = false;
        }
        else
            (*it)->Enable(false);
    }
    
    return true;
}

CBaseCFGScreen *CInstaller::CreateCFGScreen(const char *title)
{
    const int x=2, y=2, w=width()-4, h=m_pCancelButton->rely()-3;
    CCFGScreen *screen = new CCFGScreen(this, h, w, y, x);
    screen->Enable(false);
    
    if (title)
        screen->SetTitle(CreateText("<C>%s", title));
    
    return screen;
}

// -------------------------------------
// Installer base class
// -------------------------------------

void CBaseScreen::SetInfo(const char *text)
{
    m_pLabel = new CTextLabel(this, height(), width(), 0, 0, 'r');
    m_pLabel->AddText(text);
}

bool CBaseScreen::Activate(void)
{
    erase();
    if (m_bNeedDrawInit)
    {
        m_bNeedDrawInit = false;
        DrawInit();
    }
    refresh();
    return true;
}

// -------------------------------------
// Language selection screen
// -------------------------------------

bool CLangScreen::HandleEvent(CWidgetHandler *p, int type)
{
    if ((type == EVENT_CALLBACK) && (p == m_pLangMenu))
    {
        PushEvent(EVENT_CALLBACK);
        return true;
    }
    
    return false;
}

void CLangScreen::DrawInit()
{
    SetInfo("<C>Please select a language");
    
    int y = m_pLabel->rely() + m_pLabel->height() + 1;
    m_pLangMenu = new CMenu(this, height()-y, width(), y, 0, 'r');
    
    for (std::list<std::string>::iterator p=m_pInstaller->m_Languages.begin();
         p!=m_pInstaller->m_Languages.end();p++)
        m_pLangMenu->AddItem(*p);
}

// -------------------------------------
// Welcome screen
// -------------------------------------

bool CWelcomeScreen::HandleKey(chtype ch)
{
    if (CBaseScreen::HandleKey(ch))
        return true;
    
    if (ENTER(ch))
    {
        PushEvent(EVENT_CALLBACK);
        return true;
    }
    
    return false;
}

void CWelcomeScreen::DrawInit()
{
    SetInfo("<C>Welcome");
    
    int y = m_pLabel->rely() + m_pLabel->height() + 1;
    m_pTextWin = new CTextWindow(this, height()-y, width(), y, 0, true, false, 'r');
}

bool CWelcomeScreen::Activate()
{
    if (!CBaseScreen::Activate())
        return false;

    if (FileExists(m_pInstaller->GetLangWelcomeFName()))
        m_pTextWin->LoadFile(m_pInstaller->GetLangWelcomeFName());
    else if (FileExists(m_pInstaller->GetWelcomeFName()))
        m_pTextWin->LoadFile(m_pInstaller->GetWelcomeFName());
    else
        return false;
    
    return true;
}

// -------------------------------------
// License agreement screen
// -------------------------------------

bool CLicenseScreen::HandleKey(chtype ch)
{
    if (CBaseScreen::HandleKey(ch))
        return true;
    
    if (ENTER(ch))
    {
        PushEvent(EVENT_CALLBACK);
        return true;
    }
    
    return false;
}

void CLicenseScreen::DrawInit()
{
    SetInfo("<C>License agreement");
    
    int y = m_pLabel->rely() + m_pLabel->height() + 1;
    m_pTextWin = new CTextWindow(this, height()-y, width(), y, 0, true, false, 'r');
}

bool CLicenseScreen::Next()
{
    return (YesNoBox("Do you accept and understand the terms of the license agreement?"));
}

bool CLicenseScreen::Activate()
{
    if (!CBaseScreen::Activate())
        return false;

    if (FileExists(m_pInstaller->GetLangLicenseFName()))
        m_pTextWin->LoadFile(m_pInstaller->GetLangLicenseFName());
    else if (FileExists(m_pInstaller->GetLicenseFName()))
        m_pTextWin->LoadFile(m_pInstaller->GetLicenseFName());
    else
        return false;
    
    return true;
}

// -------------------------------------
// Destination directory selection screen
// -------------------------------------

bool CSelectDirScreen::HandleEvent(CWidgetHandler *p, int type)
{
    if (type == EVENT_CALLBACK)
    {
        if (p == m_pFileField)
        {
            PushEvent(EVENT_CALLBACK);
            return true;
        }
        else if (p == m_pChangeDirButton)
        {
            std::string newdir = FileDialog(m_pInstaller->m_szDestDir.c_str(), "Select destination directory");
            if (!newdir.empty())
            {
                m_pInstaller->m_szDestDir = newdir;
                m_pFileField->SetText(m_pInstaller->m_szDestDir.c_str());
            }
            return true;
        }
    }
    
    return false;
}

void CSelectDirScreen::DrawInit()
{
    const int buttonw = 22;
    SetInfo("<C>Select destination directory");
    
    int y = (height()-3)/2;
    int w = width() - (buttonw + 2);
    m_pFileField = new CInputField(this, 3, w, y, 0, 'r', 1024);
    m_pFileField->SetText(m_pInstaller->m_szDestDir.c_str());
    
    y++;
    m_pChangeDirButton = new CButton(this, 1, buttonw, y, w+2, "Select a directory", 'r');
}

bool CSelectDirScreen::Next()
{
    if (!WriteAccess(m_pInstaller->m_szDestDir))
    {
        return (ChoiceBox(m_pInstaller->GetTranslation("You don't have write permissions for this directory.\n"
                "The files can be extracted as the root user,\n"
                "but you'll need to enter the root password for this later."),
                m_pInstaller->GetTranslation("Choose another directory"),
                m_pInstaller->GetTranslation("Continue as root"), NULL) == 1);
    }
    return true;
}

// -------------------------------------
// Configuring parameters screen
// -------------------------------------

bool CCFGScreen::HandleEvent(CWidgetHandler *p, int type)
{
    return false;
}

void CCFGScreen::DrawInit()
{
    if (!m_szTitle.empty())
        SetInfo(m_szTitle.c_str());
}
