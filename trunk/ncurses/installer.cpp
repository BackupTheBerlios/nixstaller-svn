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
        
        if ((*it)->CanActivate())
        {
            (*m_CurrentScreenIt)->Enable(false);
            (*it)->Enable(true);
            ActivateChild(*it); // Give screen focus
            (*it)->Activate();
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
        
        if ((*it)->CanActivate())
        {
            (*m_CurrentScreenIt)->Enable(false);
            (*it)->Enable(true);
            ActivateChild(*it); // Give screen focus
            (*it)->Activate();
            m_CurrentScreenIt = it;
            return;
        }
    }
    
    EndProg();
}

void CInstaller::ChangeStatusText(const char *str)
{
    m_pInstallScreen->ChangeStatusText(str);
}

void CInstaller::AddInstOutput(const std::string &str)
{
    m_pInstallScreen->AppendText(str);
}

void CInstaller::SetProgress(int percent)
{
    m_pInstallScreen->SetProgress(percent);
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
    m_LuaVM.InitClass("welcomescreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(m_pWelcomeScreen, "welcomescreen", "WelcomeScreen");
    
    m_LuaVM.InitClass("licensescreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(m_pLicenseScreen, "licensescreen", "LicenseScreen");

    m_LuaVM.InitClass("selectdirscreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(m_pSelectDirScreen, "selectdirscreen", "SelectDirScreen");

    m_LuaVM.InitClass("installscreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(m_pInstallScreen, "installscreen", "InstallScreen");
    
/*    m_LuaVM.InitClass("finishscreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(new CLangScreen(this, h, w, y, x), "finishscreen", "FinishScreen");*/
    
    if (!CBaseInstall::InitLua())
        return false;
    
    return true;
}

bool CInstaller::Init(int argc, char **argv)
{
    SetTitle("Nixstaller");
    
    // Button width; longest text + 4 chars for focusing
    int bw = strlen("Cancel") + 4;

    m_pCancelButton = new CButton(this, 1, bw, height()-2, 2, "Cancel", 'r');
    m_pPrevButton = new CButton(this, 1, bw, height()-2, width()-(2*(bw+2)), "Back", 'r');
    m_pNextButton = new CButton(this, 1, bw, height()-2, width()-(bw+2), "Next", 'r');
    
    const int x=2, y=2, w=width()-4, h=m_pCancelButton->rely()-y-1;

    (m_pWelcomeScreen = new CWelcomeScreen(this, h, w, y, x))->Enable(false);
    (m_pLicenseScreen = new CLicenseScreen(this, h, w, y, x))->Enable(false);
    (m_pSelectDirScreen = new CSelectDirScreen(this, h, w, y, x))->Enable(false);
    (m_pInstallScreen = new CInstallScreen(this, h, w, y, x))->Enable(false);
    
    if (!CBaseInstall::Init(argc, argv))
        return false;
    
    m_InstallScreens.push_back(new CLangScreen(this, h, w, y, x));

    unsigned count = m_LuaVM.OpenArray("ScreenList");
    if (!count)
    {
        // Default install screens
        m_InstallScreens.push_back(m_pWelcomeScreen);
        m_InstallScreens.push_back(m_pLicenseScreen);
        m_InstallScreens.push_back(m_pSelectDirScreen);
        m_InstallScreens.push_back(m_pInstallScreen);
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
                {
                    CCFGScreen *p = (CCFGScreen *)cfgscreen;
                    
                    m_InstallScreens.push_back(p);

                    while (p && p->m_pNextScreen)
                    {
                        m_InstallScreens.push_back(p->m_pNextScreen);
                        p = p->m_pNextScreen;
                    }
                }
            }
            else
                ThrowError(false, "Wrong type found in ScreenList variabale");
        }
        m_LuaVM.CloseArray();
    }
    
    bool initscreen = true;
    for (std::list<CBaseScreen *>::iterator it=m_InstallScreens.begin(); it!=m_InstallScreens.end(); it++)
    {
        if (initscreen && (*it)->CanActivate())
        {
            ActivateChild(*it); // Give screen focus
            (*it)->Activate();
            m_CurrentScreenIt = it;
            initscreen = false;
        }
        else
            (*it)->Enable(false);
    }
    
    return true;
}

void CInstaller::Install()
{
    CBaseInstall::Install();
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
// Lua inputfield class
// -------------------------------------

CLuaInputField::CLuaInputField(CCFGScreen *owner, int y, int x, int maxx, const char *label,
                               const char *desc, const char *val, int max)
{
    int begy = y, begx = x, fieldw = maxx - begx;
    
    if (desc && *desc)
    {
        CTextLabel *pDesc = new CTextLabel(owner, 2, maxx, begy, begx, 'r');
        pDesc->AddText(desc);
        begy += pDesc->height();
    }
    
    if (label && *label)
    {
        unsigned w = Min(strlen(label), maxx/3); 
        CTextLabel *pLabel = new CTextLabel(owner, 2, w, begy, begx, 'r');
        pLabel->AddText(label);
        begx += (pLabel->width() + 1);
        fieldw -= (pLabel->width() + 1);
    }
    
    m_pInput = new CInputField(owner, 1, fieldw, begy, begx, 'r', max);
    
    if (val)
        m_pInput->SetText(val);
}

int CLuaInputField::CalcHeight(int w, const char *desc)
{
    if (desc && *desc)
        return CTextLabel::CalcHeight(w, desc) + 1;
    
    return 1;
}

// -------------------------------------
// Lua checkbox class
// -------------------------------------

CLuaCheckbox::CLuaCheckbox(CCFGScreen *owner, int y, int x, int maxx, const char *desc,
                           const std::list<std::string> &l)
{
    int begy = y;
    
    if (desc && *desc)
    {
        CTextLabel *pDesc = new CTextLabel(owner, 2, maxx, begy, x, 'r');
        pDesc->AddText(desc);
        begy += pDesc->height();
    }
    
    m_pCheckbox = new CCheckbox(owner, l.size(), maxx, begy, x, 'r');
    
    for (std::list<std::string>::const_iterator it=l.begin(); it!=l.end(); it++)
        m_pCheckbox->Add(*it);
}

int CLuaCheckbox::CalcHeight(int w, const char *desc, const std::list<std::string> &l)
{
    if (desc && *desc)
        return CTextLabel::CalcHeight(w, desc) + l.size();
    
    return l.size();
}

// -------------------------------------
// Lua radio button class
// -------------------------------------

CLuaRadioButton::CLuaRadioButton(CCFGScreen *owner, int y, int x, int maxx, const char *desc,
                                 const std::list<std::string> &l)
{
    int begy = y;
    
    if (desc && *desc)
    {
        CTextLabel *pDesc = new CTextLabel(owner, 2, maxx, begy, x, 'r');
        pDesc->AddText(desc);
        begy += pDesc->height();
    }
    
    m_pRadioButton = new CRadioButton(owner, l.size(), maxx, begy, x, 'r');
    
    for (std::list<std::string>::const_iterator it=l.begin(); it!=l.end(); it++)
        m_pRadioButton->Add(*it);
}

int CLuaRadioButton::CalcHeight(int w, const char *desc, const std::list<std::string> &l)
{
    if (desc && *desc)
        return CTextLabel::CalcHeight(w, desc) + l.size();
    
    return l.size();
}

// -------------------------------------
// Lua directory selector class
// -------------------------------------

CLuaDirSelector::CLuaDirSelector(CCFGScreen *owner, int y, int x, int maxy, int maxx, const char *desc,
                                 const char *val) : CWidgetWindow(owner, maxy, maxx, y, x, 'r', false)
{
    int begy = 0;
    
    if (desc && *desc)
    {
        CTextLabel *pDesc = new CTextLabel(this, 2, maxx, 0, 0, 'r');
        pDesc->AddText(desc);
        begy += pDesc->height();
    }
    
    const int buttonw = 20;
    
    m_pDirInput = new CInputField(this, 1, maxx-buttonw-2-x, begy, 0, 'r', 1024);
    if (val && *val)
        m_pDirInput->SetText(val);
    
    // UNDONE
    int begx = m_pDirInput->width() + 2;
    m_pDirButton = new CButton(this, 1, buttonw, begy, begx, "Browse", 'r');
}

bool CLuaDirSelector::HandleEvent(CWidgetHandler *p, int type)
{
    if ((type == EVENT_CALLBACK) && (p == m_pDirButton))
    {
        std::string dir = FileDialog(m_pDirInput->GetText().c_str(), "Select directory");
        if (!dir.empty())
            m_pDirInput->SetText(dir);
        return true;
    }
    
    return false;
}

int CLuaDirSelector::CalcHeight(int w, const char *desc)
{
    if (desc && *desc)
        return CTextLabel::CalcHeight(w, desc) + 1;
    
    return 1;
}

// -------------------------------------
// Lua config menu class
// -------------------------------------

CLuaCFGMenu::CLuaCFGMenu(CCFGScreen *owner, int y, int x, int maxy, int maxx, const char *desc) : CWidgetWindow(owner, maxy, maxx, y, x, 'r', false)
{
    int begy = 0;
    const int menuw = 20;
    const int infow = maxx - 2 - menuw;
    
    if (desc && *desc)
    {
        CTextLabel *pDesc = new CTextLabel(this, 2, maxx, 0, 0, 'r');
        pDesc->AddText(desc);
        begy += pDesc->height();
    }
    
    m_pMenu = new CMenu(this, 7, menuw, begy, 0, 'r');
    m_pInfoWindow = new CTextWindow(this, 7, infow, begy, menuw+2, false, false, 'r');
    ActivateChild(m_pMenu);
}

void CLuaCFGMenu::SetInfo()
{
    std::string item = m_pMenu->GetCurrentItemName();
    if (!item.empty() && m_Variabeles[item])
    {
        debugline("desc lines: %d('%s')\n", CTextLabel::CalcHeight(m_pInfoWindow->width(), m_Variabeles[item]->desc), m_Variabeles[item]->desc.c_str());
        m_pInfoWindow->SetText(m_Variabeles[item]->desc);
        m_pInfoWindow->refresh();
    }
}

bool CLuaCFGMenu::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pMenu)
    {
        std::string item = m_pMenu->GetCurrentItemName();
        if (!item.empty() && m_Variabeles[item])
        {
            if (type == EVENT_CALLBACK)
            {
                std::string newval;
                switch (m_Variabeles[item]->type)
                {
                    case TYPE_DIR:
                        newval = FileDialog(m_Variabeles[item]->val.c_str(), CreateText("Please select a directory for %s", item.c_str()));
                        break;
                    case TYPE_STRING:
                        newval = InputDialog(CreateText("Please enter a new value for %s", item.c_str()), m_Variabeles[item]->val.c_str());
                        break;
                    case TYPE_LIST:
                    case TYPE_BOOL:
                        newval = MenuDialog(CreateText("Please choose a new value for %s", item.c_str()), m_Variabeles[item]->options,
                                            m_Variabeles[item]->val.c_str());
                        break;
                }
                
                if (!newval.empty())
                {
                    m_Variabeles[item]->val = newval;
                    SetInfo();
                }
            }
            else if (type == EVENT_DATACHANGED)
            {
                SetInfo();
            }
            else
                return false;
        }
        return true;
    }
    return false;
}

void CLuaCFGMenu::AddVar(const char *name, const char *desc, const char *val, EVarType type, std::list<std::string> *l)
{
    CBaseLuaCFGMenu::AddVar(name, desc, val, type, l);
    m_pMenu->AddItem(name);
    
    SetInfo();
}

int CLuaCFGMenu::CalcHeight(int w, const char *desc)
{
    if (desc && *desc)
        return CTextLabel::CalcHeight(w, desc) + 7;
    
    return 7;
}

// -------------------------------------
// Installer base screen class
// -------------------------------------

void CBaseScreen::SetInfo(const char *text)
{
    m_pLabel = new CTextLabel(this, height(), width(), 0, 0, 'r');
    m_pLabel->AddText(text);
}

void CBaseScreen::Activate(void)
{
    erase();
    if (m_bNeedDrawInit)
    {
        m_bNeedDrawInit = false;
        DrawInit();
    }
    refresh();
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

bool CWelcomeScreen::CanActivate()
{
    if (!CBaseScreen::CanActivate())
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

bool CLicenseScreen::CanActivate()
{
    if (!CBaseScreen::CanActivate())
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
    m_pFileField = new CInputField(this, 1, w, y, 0, 'r', 1024);
    m_pFileField->SetText(m_pInstaller->m_szDestDir.c_str());
    
    m_pChangeDirButton = new CButton(this, 1, buttonw, y, w+2, "<C>Select a directory", 'r');
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
// Install screen
// -------------------------------------

void CInstallScreen::DrawInit()
{
    m_pProgLabel = new CTextLabel(this, 1, width(), 0, 0, 'r');
    m_pProgLabel->AddText("<C>Progress");
    
    int x=2, y=1;

    m_pProgressbar = new CProgressbar(this, 3, width()-x-2, y, x, 1, 100, 'r');
    
    y += 3;
    
    m_pStatLabel = new CTextLabel(this, 1, width(), y, 0, 'r');
    
    y++;
    
    m_pTextWin = new CTextWindow(this, height()-y, width()-x-2, y, x, true, true, 'r');
}

void CInstallScreen::ChangeStatusText(const char *txt)
{
    m_pStatLabel->SetText(CreateText("<C>%s", txt));
    m_pStatLabel->refresh();
}

void CInstallScreen::Activate()
{
    CBaseScreen::Activate();
    m_pInstaller->Install();
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

void CCFGScreen::Activate()
{
    CBaseScreen::Activate();
    
    if (!m_ChildList.empty())
    {
        for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
        {
            if ((*it)->Enabled() && (*it)->CanFocus())
            {
                ActivateChild(*it);
                break;
            }
        }
    }
}

CBaseLuaInputField *CCFGScreen::CreateInputField(const char *label, const char *desc, const char *val, int max)
{
    int h = CLuaInputField::CalcHeight(width()-3, desc);
    
    if ((h + m_iStartY) < height())
    {
        CLuaInputField *field = new CLuaInputField(this, m_iStartY, 1, width()-3, label, desc, val, max);
        m_iStartY += (h + 1);
        return field;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pInstaller->CreateCFGScreen(m_szTitle.c_str());
    
    return m_pNextScreen->CreateInputField(label, desc, val, max);
}

CBaseLuaCheckbox *CCFGScreen::CreateCheckbox(const char *desc, const std::list<std::string> &l)
{
    int h = CLuaCheckbox::CalcHeight(width()-3, desc, l);
    
    if ((h + m_iStartY) < height())
    {
        CLuaCheckbox *box = new CLuaCheckbox(this, m_iStartY, 1, width()-3, desc, l);
        m_iStartY += (h + 1);
        return box;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pInstaller->CreateCFGScreen(m_szTitle.c_str());
    
    return m_pNextScreen->CreateCheckbox(desc, l);
}

CBaseLuaRadioButton *CCFGScreen::CreateRadioButton(const char *desc, const std::list<std::string> &l)
{
    int h = CLuaRadioButton::CalcHeight(width()-3, desc, l);
    
    if ((h + m_iStartY) <= height())
    {
        CLuaRadioButton *radio = new CLuaRadioButton(this, m_iStartY, 1, width()-3, desc, l);
        m_iStartY += h;
        return radio;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pInstaller->CreateCFGScreen(m_szTitle.c_str());
    
    return m_pNextScreen->CreateRadioButton(desc, l);
}

CBaseLuaDirSelector *CCFGScreen::CreateDirSelector(const char *desc, const char *val)
{
    int h = CLuaDirSelector::CalcHeight(width()-3, desc);
    
    if ((h + m_iStartY) <= height())
    {
        CLuaDirSelector *sel = new CLuaDirSelector(this, m_iStartY, 1, h, width()-3, desc, val);
        m_iStartY += h;
        return sel;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pInstaller->CreateCFGScreen(m_szTitle.c_str());
    
    return m_pNextScreen->CreateDirSelector(desc, val);
}

CBaseLuaCFGMenu *CCFGScreen::CreateCFGMenu(const char *desc)
{
    int h = CLuaCFGMenu::CalcHeight(width()-3, desc);
    
    if ((h + m_iStartY) <= height())
    {
        CLuaCFGMenu *menu = new CLuaCFGMenu(this, m_iStartY, 1, h, width()-3, desc);
        m_iStartY += h;
        return menu;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pInstaller->CreateCFGScreen(m_szTitle.c_str());
    
    return m_pNextScreen->CreateCFGMenu(desc);
}

