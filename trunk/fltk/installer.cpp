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

#include "fltk.h"

// -------------------------------------
// Main installer screen
// -------------------------------------

bool CInstaller::InitLua()
{
    m_LuaVM.InitClass("welcomescreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(m_pWelcomeScreen, "welcomescreen", "WelcomeScreen");
    
    m_LuaVM.InitClass("licensescreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(m_pLicenseScreen, "licensescreen", "LicenseScreen");

    m_LuaVM.InitClass("selectdirscreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(m_pSelectDirScreen, "selectdirscreen", "SelectDirScreen");

    m_LuaVM.InitClass("installscreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(m_pInstallFilesScreen, "installscreen", "InstallScreen");
    
    m_LuaVM.InitClass("finishscreen");
    m_LuaVM.RegisterUData<CBaseScreen *>(m_pFinishScreen, "finishscreen", "FinishScreen");
    
    if (!CBaseInstall::InitLua())
        return false;
    
    return true;
}

bool CInstaller::Init(int argc, char **argv)
{
    m_pMainWindow = new Fl_Window(MAIN_WINDOW_W, MAIN_WINDOW_H, "Nixstaller");
    m_pMainWindow->callback(WizCancelCB);

    m_pCancelButton = new Fl_Button(20, (MAIN_WINDOW_H-30), 120, 25, "Cancel");
    m_pCancelButton->callback(WizCancelCB, this);
    m_pPrevButton = new Fl_Button(MAIN_WINDOW_W-280, (MAIN_WINDOW_H-30), 120, 25, "@<-    Back");
    m_pPrevButton->callback(WizPrevCB, this);
    m_pPrevButton->deactivate(); // First screen, no go back button yet
    m_pNextButton = new Fl_Button(MAIN_WINDOW_W-140, (MAIN_WINDOW_H-30), 120, 25, "Next    @->");
    m_pNextButton->callback(WizNextCB, this);
    m_pAboutButton = new Fl_Button((MAIN_WINDOW_W-80), 5, 60, 12, "About");
    m_pAboutButton->labelsize(10);
    m_pAboutButton->callback(ShowAboutCB, this);

    m_pWelcomeScreen = new CWelcomeScreen(this);
    m_pLicenseScreen = new CLicenseScreen(this);
    m_pSelectDirScreen = new CSelectDirScreen(this);
    m_pInstallFilesScreen = new CInstallFilesScreen(this);
    m_pFinishScreen = new CFinishScreen(this);
    
    m_pWizard = new Fl_Wizard(20, 20, (MAIN_WINDOW_W-40), (MAIN_WINDOW_H-60), NULL);
    m_pWizard->end();

    if (!CBaseInstall::Init(argc, argv))
        return false;

//     m_pWizard = new Fl_Wizard(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    
    Fl_Group *group;

    CBaseScreen *lang = new CLangScreen(this);
    if ((group = lang->Create()))
    {
        m_pWizard->add(group);
        m_ScreenList.push_back(lang);
    }
    
    UpdateLanguage();

    unsigned count = m_LuaVM.OpenArray("ScreenList");
    
    if (!count)
    {
        // Default screens

        if ((group = m_pWelcomeScreen->Create()))
        {
            m_pWizard->add(group);
            m_ScreenList.push_back(m_pWelcomeScreen);
        }
        
        if ((group = m_pLicenseScreen->Create()))
        {
            m_pWizard->add(group);
            m_ScreenList.push_back(m_pLicenseScreen);
        }

        if ((group = m_pSelectDirScreen->Create()))
        {
            m_pWizard->add(group);
            m_ScreenList.push_back(m_pSelectDirScreen);
        }

        if ((group = m_pInstallFilesScreen->Create()))
        {
            m_pWizard->add(group);
            m_ScreenList.push_back(m_pInstallFilesScreen);
        }

        if ((group = m_pFinishScreen->Create()))
        {
            m_pWizard->add(group);
            m_ScreenList.push_back(m_pFinishScreen);
        }
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
                {
                    if ((group = screen->Create()))
                    {
                        m_pWizard->add(group);
                        m_ScreenList.push_back(screen);
                    }
                }
                else if (cfgscreen)
                {
                    CCFGScreen *p = (CCFGScreen *)cfgscreen;
                    
                    if ((group = p->Create()))
                    {
                        m_pWizard->add(group);
                        m_ScreenList.push_back(p);

                        int cnt = 0;
                        while (p)
                        {
                            p = p->m_pNextScreen;
                            cnt++;
                        }
                        
                        p = (CCFGScreen *)cfgscreen;
                        
                        if (cnt > 1)
                            p->SetCounter(1, cnt);
    
                        p = p->m_pNextScreen;
                        int cur = 2; // We start at first linked screen
    
                        while (p)
                        {
                            if (cnt > 1)
                                p->SetCounter(cur, cnt);
                            
                            if ((group = p->Create()))
                            {
                                m_pWizard->add(group);
                                m_ScreenList.push_back(p);
                            }

                            p = p->m_pNextScreen;
                            cur++;
                        }
                    }
                }
            }
            else
                ThrowError(false, "Wrong type found in ScreenList variabale");
        }
        m_LuaVM.CloseArray();
    }
    
    m_pWizard->end();
    
    if (!m_ScreenList.front()->Activate())
        Next();
    
    m_pMainWindow->end();
    return true;
}

CInstaller::~CInstaller()
{
    for(std::list<CBaseScreen *>::iterator p=m_ScreenList.begin();p!=m_ScreenList.end();p++)
        delete *p;
}

void CInstaller::ChangeStatusText(const char *str)
{
    m_pInstallFilesScreen->ChangeStatusText(str);
}

void CInstaller::AddInstOutput(const std::string &str)
{
    m_pInstallFilesScreen->AppendText(str);
}

void CInstaller::SetProgress(int percent)
{
    m_pInstallFilesScreen->SetProgress(percent);
}
    
void CInstaller::Install()
{
    m_bInstallFiles = true;
    m_pPrevButton->deactivate();
    m_pNextButton->deactivate();
    
    CBaseInstall::Install();
    
    m_bInstallFiles = false;
    m_pCancelButton->deactivate();
    m_pNextButton->activate();
    
    if (!FileExists(GetFinishFName()) && !FileExists(GetLangFinishFName()))
        m_pNextButton->label(GetTranslation("Finish"));
}

void CInstaller::UpdateLanguage(void)
{
    CFLTKBase::UpdateLanguage();
    
    // Update main buttons
    m_pCancelButton->label(GetTranslation("Cancel"));
    m_pPrevButton->label(CreateText("@<-    %s", GetTranslation("Back")));
    m_pNextButton->label(CreateText("%s    @->", GetTranslation("Next")));
    
    // Update all screens
    for(std::list<CBaseScreen *>::iterator p=m_ScreenList.begin();p!=m_ScreenList.end();p++)
        (*p)->UpdateLang();
}

void CInstaller::WizCancelCB(Fl_Widget *, void *p)
{
    CInstaller *pInst = (CInstaller *)p;
    
    char *msg;
    if (pInst->m_bInstallFiles)
        msg = GetTranslation("Install commands are still running\n"
                "If you abort now this may lead to a broken installation\n"
                "Are you sure?");
    else
        msg = GetTranslation("This will abort the installation\nAre you sure?");
    
    if (fl_choice(msg, GetTranslation("No"), GetTranslation("Yes"), NULL))
        EndProg();
}

CBaseCFGScreen *CInstaller::CreateCFGScreen(const char *title)
{
    CCFGScreen *scr = new CCFGScreen(this, title);
    m_pWizard->add(scr->Create());
    return scr;
}

void CInstaller::Prev()
{
    for(std::list<CBaseScreen *>::iterator p=m_ScreenList.begin();p!=m_ScreenList.end();p++)
    {
        if ((*p)->GetGroup() == m_pWizard->value())
        {
            if (p == m_ScreenList.begin()) break;
            if ((*p)->Prev())
            {
                p--;
                m_pWizard->prev();
                while (!(*p)->Activate() && (p != m_ScreenList.begin())) { m_pWizard->prev(); p--; }
            }
            break;
        }
    }
}

void CInstaller::Next()
{
    for(std::list<CBaseScreen *>::iterator p=m_ScreenList.begin();p!=m_ScreenList.end();p++)
    {
        if ((*p)->GetGroup() == m_pWizard->value())
        {
            if (p == m_ScreenList.end()) break;
            if ((*p)->Next())
            {
                p++;
                m_pWizard->next();
                while ((p != m_ScreenList.end()) && (!(*p)->Activate())) { m_pWizard->next(); p++; }

                if (p == m_ScreenList.end())
                    EndProg();
            }
            break;
        }
    }
}

// -------------------------------------
// Base FLTK Lua Widget class
// -------------------------------------

CBaseLuaWidget::CBaseLuaWidget(int x, int y, int w, int h, const char *desc) : m_iStartX(x), m_iStartY(y), m_iWidth(w), m_iHeight(h), m_pBox(NULL)
{
    if (desc && *desc)
        m_szDescription = desc;
}

void CBaseLuaWidget::MakeTitle()
{
    if (m_szDescription.empty())
        return;
    
    const char *desc = MakeTranslation(m_szDescription);
    
    if (!m_pBox)
    {
        m_pBox = new Fl_Box(m_pGroup->x(), m_pGroup->y(), m_iWidth, TitleHeight(m_iWidth, desc), desc);
        m_pBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_WRAP);
    }
    else
        m_pBox->label(desc);
}

Fl_Group *CBaseLuaWidget::Create()
{
    m_pGroup = new Fl_Group(m_iStartX, m_iStartY, m_iWidth, m_iHeight);
    m_pGroup->begin();
    
    if (!m_szDescription.empty())
        MakeTitle();
    
    m_pGroup->end();
    
    return m_pGroup;
}

void CBaseLuaWidget::UpdateLanguage()
{
    MakeTitle();
}

int CBaseLuaWidget::TitleHeight(int w, const char *desc)
{
    if (desc && *desc)
    {
        if (fl_width(desc) > w)
            return 40; // We allow max 2 lines UNDONE ?
        else
            return 20;
    }
    
    return 0;
}

// -------------------------------------
// Lua inputfield class
// -------------------------------------

int CLuaInputField::m_iFieldHeight = 20;

CLuaInputField::CLuaInputField(int x, int y, int w, int h, const char *label, const char *desc,
                               const char *val, int max, const char *type) : CBaseLuaInputField(type), CBaseLuaWidget(x, y, w, h, desc),
                                                                             m_pLabel(NULL), m_iMax(max)
{
    if (label && *label)
        m_szLabel = label;
    
    if (val && *val)
        m_szValue = val;
}

Fl_Group *CLuaInputField::Create()
{
    Fl_Group *group = CBaseLuaWidget::Create();
    int x = group->x(), y = group->y() + DescHeight(), w = (((float)m_pGroup->w() / 100.0f) * (float)GetDefaultSpacing());
    
    if (!m_szLabel.empty())
    {
        group->add(m_pLabel = new Fl_Box(x, y, w, m_iFieldHeight, MakeTranslation(m_szLabel)));
        m_pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    }
    
    x += w + 10;
    w = m_pGroup->w() - (x - m_pGroup->x());
    
    if (GetType() == "number")
        group->add(m_pInput = new Fl_Int_Input(x, y, w, m_iFieldHeight));
    else if (GetType() == "float")
        group->add(m_pInput = new Fl_Float_Input(x, y, w, m_iFieldHeight));
    else
        group->add(m_pInput = new Fl_Input(x, y, w, m_iFieldHeight));
    
    m_pInput->maximum_size(m_iMax);
    
    if (!m_szValue.empty())
        m_pInput->value(m_szValue.c_str());
    
    return group;
}

void CLuaInputField::UpdateLanguage()
{
    CBaseLuaWidget::UpdateLanguage();
    
    if (m_pLabel)
        m_pLabel->label(MakeTranslation(m_szLabel));
}

void CLuaInputField::SetSpacing(int percent)
{
    int x = m_pGroup->x(), w = (((float)m_pGroup->w() / 100.0f) * (float)percent);
    
    if (m_pLabel)
    {
        m_pLabel->resize(x, m_pLabel->y(), w, m_pLabel->h());
        m_pLabel->redraw();
    }
    
    x += w + 10;
    w = m_pGroup->w() - (x - m_pGroup->x());
    
    m_pInput->resize(x, m_pInput->y(), w, m_pGroup->h());
    m_pInput->redraw();
}

int CLuaInputField::CalcHeight(int w, const char *desc)
{
    return m_iFieldHeight + TitleHeight(w, desc);
}

// -------------------------------------
// Lua checkbox class
// -------------------------------------

int CLuaCheckbox::m_iButtonHeight = 20;
int CLuaCheckbox::m_iButtonSpace = 5;
int CLuaCheckbox::m_iBoxSpace = 10;

CLuaCheckbox::CLuaCheckbox(int x, int y, int w, int h, const char *desc,
                           const std::vector<std::string> &l) : CBaseLuaWidget(x, y, w, h, desc), m_Options(l)
{
}

Fl_Group *CLuaCheckbox::Create()
{
    Fl_Group *group = CBaseLuaWidget::Create();
    int x = group->x(), y = group->y() + DescHeight(), w = 0;
    
    // Find longest text
    for (std::vector<std::string>::const_iterator it=m_Options.begin(); it!=m_Options.end(); it++)
        w = Max(w, 40 + fl_width(it->c_str()));

    group->box(FL_ENGRAVED_BOX);
    group->size(w, group->h());
    
    x += 5;
    y += 5;

    for (std::vector<std::string>::const_iterator it=m_Options.begin(); it!=m_Options.end(); it++)
    {
        Fl_Check_Button *button = new Fl_Check_Button(x, y, 60, m_iButtonHeight, MakeTranslation(*it));
        button->type(FL_TOGGLE_BUTTON);
        m_Buttons.push_back(button);
        group->add(button);
        y += (m_iButtonHeight + m_iButtonSpace);
    }
    
    return group;
}

void CLuaCheckbox::UpdateLanguage()
{
    CBaseLuaWidget::UpdateLanguage();
    
    for (unsigned u=0; u<m_Buttons.size(); u++)
        m_Buttons[u]->label(MakeTranslation(m_Options[u]));
}

int CLuaCheckbox::CalcHeight(int w, const char *desc, const std::vector<std::string> &l)
{
    if (!l.empty())
        return (m_iButtonHeight * l.size()) + (m_iButtonSpace * (l.size()-1)) + TitleHeight(w, desc) + m_iBoxSpace;
    else
        return TitleHeight(w, desc);
}

// -------------------------------------
// Lua checkbox class
// -------------------------------------

int CLuaRadioButton::m_iButtonHeight = 20;
int CLuaRadioButton::m_iButtonSpace = 5;
int CLuaRadioButton::m_iBoxSpace = 10;

CLuaRadioButton::CLuaRadioButton(int x, int y, int w, int h, const char *desc,
                                 const std::vector<std::string> &l) : CBaseLuaWidget(x, y, w, h, desc), m_Options(l)
{
}

Fl_Group *CLuaRadioButton::Create()
{
    Fl_Group *group = CBaseLuaWidget::Create();
    int x = group->x(), y = group->y() + DescHeight(), w = 0;
    
    // Find longest text
    for (std::vector<std::string>::const_iterator it=m_Options.begin(); it!=m_Options.end(); it++)
        w = Max(w, 40 + fl_width(it->c_str()));

    group->box(FL_ENGRAVED_BOX);
    group->size(w, group->h());
    
    x += 5;
    y += 5;
    
    for (std::vector<std::string>::const_iterator it=m_Options.begin(); it!=m_Options.end(); it++)
    {
        Fl_Round_Button *button = new Fl_Round_Button(x, y, w, m_iButtonHeight, MakeTranslation(*it));
        button->type(FL_RADIO_BUTTON);
        m_Buttons.push_back(button);
        group->add(button);
        y += (m_iButtonHeight + m_iButtonSpace);
    }
    
    if (!m_Buttons.empty())
        m_Buttons.front()->setonly(); // Enable first button
    
    return group;
}

void CLuaRadioButton::UpdateLanguage()
{
    CBaseLuaWidget::UpdateLanguage();
    
    for (unsigned u=0; u<m_Buttons.size(); u++)
        m_Buttons[u]->label(MakeTranslation(m_Options[u]));
}

int CLuaRadioButton::EnabledButton()
{
    for (unsigned u=0; u<m_Buttons.size(); u++)
    {
        if (m_Buttons[u]->value())
            return u;
    }
    
    return -1; // UNDONE: Exception?
}

int CLuaRadioButton::CalcHeight(int w, const char *desc, const std::vector<std::string> &l)
{
    if (!l.empty())
        return (m_iButtonHeight * l.size()) + (m_iButtonSpace * (l.size()-1)) + TitleHeight(w, desc) + m_iBoxSpace;
    else
        return TitleHeight(w, desc);
}

// -------------------------------------
// Lua directory selector class
// -------------------------------------

int CLuaDirSelector::m_iFieldHeight = 35;
int CLuaDirSelector::m_iButtonWidth = 120, CLuaDirSelector::m_iButtonHeight = 25;

CLuaDirSelector::CLuaDirSelector(int x, int y, int w, int h, const char *desc,
                                 const char *val) : CBaseLuaWidget(x, y, w, h, desc), m_pDirChooser(NULL)
{
    if (val && *val)
        m_szValue = val;
}

void CLuaDirSelector::OpenDirChooser(void)
{
    m_pDirChooser->directory(GetFirstValidDir(m_pDirInput->value()).c_str());
    m_pDirChooser->show();
    
    while(m_pDirChooser->visible())
        Fl::wait();

    const char *dir = m_pDirChooser->value();
    
    if (!dir || !dir[0])
        return;
        
    m_pDirInput->value(dir);
}

Fl_Group *CLuaDirSelector::Create()
{
    Fl_Group *group = CBaseLuaWidget::Create();
    int x = group->x(), y = group->y() + DescHeight(), w = group->w() - x - m_iButtonWidth - 20;
    
    group->add(m_pDirInput = new Fl_File_Input(x, y, w, m_iFieldHeight));
    
    if (!m_szValue.empty())
        m_pDirInput->value(m_szValue.c_str());
    
    x = group->w() - m_iButtonWidth;
    y += (m_iFieldHeight - m_iButtonHeight) / 2; // Center between inputfield
    group->add(m_pDirButton = new Fl_Button(x, y, m_iButtonWidth, m_iButtonHeight, "Browse"));
    m_pDirButton->callback(OpenDirSelWinCB, this);
    
    return group;
}

void CLuaDirSelector::UpdateLanguage()
{
    CBaseLuaWidget::UpdateLanguage();
    m_pDirButton->label(GetTranslation("Browse"));
    
    // Dir chooser needs to be recreated
    if (m_pDirChooser)
        delete m_pDirChooser;
    
    m_pDirChooser = new Fl_File_Chooser(m_szValue.c_str(), "*",
                                        (Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE),
                                        GetTranslation("Select directory"));
    m_pDirChooser->preview(false);
    m_pDirChooser->previewButton->hide();
    m_pDirChooser->newButton->tooltip(Fl_File_Chooser::new_directory_tooltip);
}

int CLuaDirSelector::CalcHeight(int w, const char *desc)
{
    return m_iFieldHeight + TitleHeight(w, desc);
}

// -------------------------------------
// Lua config menu class
// -------------------------------------

int CLuaCFGMenu::m_iMenuHeight = 140;
int CLuaCFGMenu::m_iMenuWidth = 220;
int CLuaCFGMenu::m_iDescHeight = 110;
int CLuaCFGMenu::m_iButtonWidth = 140;
int CLuaCFGMenu::m_iButtonHeight = 25;

CLuaCFGMenu::CLuaCFGMenu(int x, int y, int w, int h, const char *desc) : CBaseLuaWidget(x, y, w, h, desc), m_pDirChooser(NULL)
{
}

void CLuaCFGMenu::CreateDirSelector()
{
    if (m_pDirChooser)
        delete m_pDirChooser;
    
    m_pDirChooser = new Fl_File_Chooser("~", "*",
                                        (Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE),
                                        GetTranslation("Select a directory"));
    m_pDirChooser->preview(false);
    m_pDirChooser->previewButton->hide();
    m_pDirChooser->newButton->tooltip(Fl_File_Chooser::new_directory_tooltip);
}

void CLuaCFGMenu::SetInfo()
{
    if (!m_pMenu->size())
        return;
    
    const char *item = m_pMenu->text(m_pMenu->value());
    if (item && *item && m_Variabeles[item])
        m_pDescBuffer->text(GetTranslation(m_Variabeles[item]->desc.c_str()));
    else
        m_pDescBuffer->text(NULL);
}

void CLuaCFGMenu::SetInputMethod()
{
    const char *item = m_pMenu->text(m_pMenu->value());
    if (!item || !*item || !m_Variabeles[item])
        return;
    
    m_pBrowseButton->hide();
    m_pChoiceMenu->hide();
    m_pInputField->hide();
    
    switch(m_Variabeles[item]->type)
    {
        case TYPE_DIR:
            m_pBrowseButton->show();
            break;
        case TYPE_LIST:
        case TYPE_BOOL:
        {
            m_pChoiceMenu->clear();
            
            int cur = 0, n = 0;
            for (TOptionsType::iterator it=m_Variabeles[item]->options.begin(); it!=m_Variabeles[item]->options.end(); it++)
            {
                m_pChoiceMenu->add(it->c_str());
                if (!cur && (*it == m_Variabeles[item]->val))
                    cur = n;
                n++;
            }
            
            if (m_pChoiceMenu->size())
                m_pChoiceMenu->value(cur);
            
            m_pChoiceMenu->show();
            break;
        }
        case TYPE_STRING:
            m_pInputField->value(m_Variabeles[item]->val.c_str());
            m_pInputField->show();
            break;
    }
}

void CLuaCFGMenu::OpenDir()
{
    const char *item = m_pMenu->text(m_pMenu->value());
    if (!item || !*item || !m_Variabeles[item])
        return;
    
    m_pDirChooser->directory(GetFirstValidDir(m_Variabeles[item]->val).c_str());
    m_pDirChooser->show();
    while(m_pDirChooser->visible())
        Fl::wait();
    
    const char *newdir = m_pDirChooser->value();
    if (newdir && *newdir)
        m_Variabeles[item]->val = newdir;
}

void CLuaCFGMenu::SetString(const char *s)
{
    const char *item = m_pMenu->text(m_pMenu->value());
    if (!item || !*item || !m_Variabeles[item])
        return;
    
    m_Variabeles[item]->val = s;
}

void CLuaCFGMenu::SetChoice(int n)
{
    const char *item = m_pMenu->text(m_pMenu->value());
    if (!item || !*item || !m_Variabeles[item])
        return;
    
    m_Variabeles[item]->val = m_Variabeles[item]->options[n];
}

Fl_Group *CLuaCFGMenu::Create()
{
    Fl_Group *group = CBaseLuaWidget::Create();
    int x = group->x(), y = group->y() + DescHeight();
    
    group->add(m_pMenu = new Fl_Hold_Browser(x, y, m_iMenuWidth, m_iMenuHeight));
    m_pMenu->callback(MenuCB, this);
    
    x += m_iMenuWidth + 20;
    int w = group->w() - (x - group->x());
    
    group->add(m_pDescOutput = new Fl_Text_Display(x, y, w, m_iDescHeight));
    m_pDescOutput->buffer(m_pDescBuffer = new Fl_Text_Buffer);
    
    x += ((m_pGroup->w() - (x - m_pGroup->x())) - m_iButtonWidth)/2;
    y += m_iDescHeight + 10;
    group->add(m_pBrowseButton = new Fl_Button(x, y, m_iButtonWidth, m_iButtonHeight, "Modify"));
    m_pBrowseButton->callback(BrowseCB, this);
    
    x = m_pDescOutput->x();
    x += ((m_pGroup->w() - (x - m_pGroup->x())) - m_iButtonWidth)/2;
    group->add(m_pChoiceMenu = new Fl_Choice(x, y, m_iButtonWidth, m_iButtonHeight));
    m_pChoiceMenu->when(FL_WHEN_CHANGED);
    m_pChoiceMenu->callback(ChoiceCB, this);
    
    x = m_pDescOutput->x() + 10;
    w = group->w() - (x - group->x());
    group->add(m_pInputField = new Fl_Input(x, y, w, m_iButtonHeight));
    m_pInputField->when(FL_WHEN_CHANGED);
    m_pInputField->callback(InputFieldCB, this);

    CreateDirSelector();
    
    return group;
}

void CLuaCFGMenu::UpdateLanguage()
{
    CBaseLuaWidget::UpdateLanguage();
    m_pBrowseButton->label(GetTranslation("Browse"));
    CreateDirSelector(); // Recreate, so that it will use the new translations
    SetInfo();
}

void CLuaCFGMenu::AddVar(const char *name, const char *desc, const char *val, EVarType type, TOptionsType *l)
{
    CBaseLuaCFGMenu::AddVar(name, desc, val, type, l);
    m_pMenu->add(name);
    
    if (m_pMenu->size() == 1) // Init menu: highlight first item
    {
        m_pMenu->value(1);
        SetInfo();
        SetInputMethod();
    }
}

int CLuaCFGMenu::CalcHeight(int w, const char *desc)
{
    return m_iMenuHeight + TitleHeight(w, desc);
}

// -------------------------------------
// Base install screen
// -------------------------------------

// Used when label is aligned outside widget.
int CBaseScreen::CenterX(int w, const char *label, bool left)
{
    int textw = fl_width(label);
    int ret = ((MAIN_WINDOW_W-(w+textw)) / 2);
    if (left) // if the label is left aligned the x pos should be adjusted so it starts after the label
        ret += textw;
    
    return ret;
}

int CBaseScreen::CenterX2(int w, const char *l1, const char *l2, bool left1, bool left2)
{
    int t1 = fl_width(l1), t2 = fl_width(l2);
    int ret = ((MAIN_WINDOW_W-w-t1-t2) / 2);
    
    if (left1) // if the label is left aligned the x pos should be adjusted so it starts after the label
        ret += t1;
    
    if (left2)
        ret += t2;
    
    return ret;
}
// -------------------------------------
// Language selection screen
// -------------------------------------

Fl_Group *CLangScreen::Create(void)
{
    CBaseScreen::Create();

//     m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    
    m_pTitleBox = new Fl_Box(CenterX(260), 40, 260, 100, "Please select a language");
    
    m_pChoiceMenu = new Fl_Choice(CenterX(120, "Language: ", true), CenterY(25), 120, 25, "Language: ");
    m_pChoiceMenu->callback(LangMenuCB, this);
    
    for (std::vector<std::string>::iterator p=m_pOwner->m_Languages.begin();
         p!=m_pOwner->m_Languages.end();p++)
        m_pChoiceMenu->add(MakeCString(*p));

    m_pChoiceMenu->value(0);
    
    m_pGroup->end();
        
    return m_pGroup;
}

bool CLangScreen::Next()
{
    m_pOwner->m_pPrevButton->activate();
    m_pOwner->UpdateLanguage();
    return true;
}

bool CLangScreen::Activate()
{
    if (m_pOwner->m_Languages.size() <= 1) // Nothing to select...
        return false;
    
    unsigned size = m_pOwner->m_Languages.size();
    for (unsigned u=0; u<size; u++)
    {
        if (m_pOwner->m_Languages[u] == m_pOwner->m_szCurLang)
        {
            m_pChoiceMenu->value(u);
            break;
        }
    }
    
    return true;
}

void CLangScreen::UpdateLang()
{
    m_pTitleBox->label(GetTranslation("Please select a language"));
    
    const char *label = CreateText("%s: ", GetTranslation("Language"));
    m_pChoiceMenu->label(label);
    
    // Re center
    m_pChoiceMenu->position(CenterX(120, label, true), m_pChoiceMenu->y());
    m_pChoiceMenu->redraw();
}

// -------------------------------------
// Welcome text display screen
// -------------------------------------

Fl_Group *CWelcomeScreen::Create(void)
{
    CBaseScreen::Create();
    
    Fl::visual(FL_RGB);
    
//     m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);

    int x = m_pGroup->x() + 20, y = m_pGroup->y() + 40, h = m_pGroup->h()-(y-m_pGroup->y())-20;
    m_pImage = Fl_Shared_Image::get(m_pOwner->GetIntroPicFName());
    if (m_pImage)
    {
        m_pImageBox = new Fl_Box(x, y, 165, h);

        // Scale image if its to big
        if (m_pImage->w() > m_pImageBox->w() || m_pImage->h() > m_pImageBox->h())
        {
            int imgw, imgh;
            float wfact, hfact;

            // Check which scale factor is the biggest and use that one. This makes sure that the image will fit.
            wfact = (float(m_pImage->w())/float(m_pImageBox->w()));
            hfact = (float(m_pImage->h())/float(m_pImageBox->h()));

            if (wfact >= hfact)
            {
                imgw = m_pImageBox->w();
                imgh = (int)(float(m_pImage->h()) / wfact);
            }
            else
            {
                imgw = (int)(float(m_pImage->w()) / hfact);
                imgh = m_pImageBox->h();
            }

            Fl_Image *temp = m_pImage->copy(imgw, imgh);
            m_pImage->release();
            m_pImage = (Fl_Shared_Image *)temp;
        }
        else
        {
            // Make box smaller if image was smaller than the box and center the y axis.
            m_pImageBox->resize(m_pImageBox->x(), 60 + ((m_pImageBox->h() - m_pImage->h())/2), m_pImage->w(), m_pImage->h());
        }
        
        m_pImageBox->image(m_pImage);
        m_pImageBox->align(FL_ALIGN_CENTER);

        x += m_pImageBox->w() + 20;
        m_pDisplay = new Fl_Text_Display(x, y, m_pGroup->w()-(x-m_pGroup->x())-20, h, "Welcome");
    }
    else
    {
        m_pDisplay = new Fl_Text_Display(x, y, m_pGroup->w()-(x-m_pGroup->x())-20, m_pGroup->h()-(y-m_pGroup->y())-20, "Welcome");
    }
    
    m_pBuffer = new Fl_Text_Buffer;
    m_pDisplay->buffer(m_pBuffer);
    
    m_pGroup->end();
    
    return m_pGroup;
}

void CWelcomeScreen::UpdateLang()
{
    m_bHasText = (!m_pBuffer->loadfile(m_pOwner->GetLangWelcomeFName()) ||
                  !m_pBuffer->loadfile(m_pOwner->GetWelcomeFName()));
    m_pDisplay->label(GetTranslation("Welcome"));
}

// -------------------------------------
// License agreement screen
// -------------------------------------

Fl_Group *CLicenseScreen::Create(void)
{
    CBaseScreen::Create();

//     m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    
    m_pBuffer = new Fl_Text_Buffer;
    
    int y = m_pGroup->y()+20, h = m_pGroup->h()-(y-m_pGroup->y())-60;
    m_pDisplay = new Fl_Text_Display(m_pGroup->x()+20, y, m_pGroup->w()-m_pGroup->x()-20, h, "License agreement");
    m_pDisplay->buffer(m_pBuffer);
    
    y += h + 20;
    const char *txt = "I Agree to this license agreement";
    m_pCheckButton = new Fl_Check_Button(CenterX(50, txt, false), y, 50, 25, txt);
    m_pCheckButton->callback(LicenseCheckCB, this);
    
    m_pGroup->end();
    return m_pGroup;
}

void CLicenseScreen::UpdateLang()
{
    m_bHasText = (!m_pBuffer->loadfile(m_pOwner->GetLangLicenseFName()) ||
            !m_pBuffer->loadfile(m_pOwner->GetLicenseFName()));
    
    m_pDisplay->label(GetTranslation("License agreement"));
    m_pCheckButton->label(GetTranslation("I Agree to this license agreement"));
}

bool CLicenseScreen::Activate()
{
    if (!m_bHasText) return false;
    
    if (!m_pCheckButton->value())
        m_pOwner->m_pNextButton->deactivate();
    
    return true;
}

// -------------------------------------
// Destination dir selector screen
// -------------------------------------

Fl_Group *CSelectDirScreen::Create()
{
    CBaseScreen::Create();
    
//     m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);

    m_pBox = new Fl_Box(CenterX(260), 40, 260, 100, "Select destination directory");
    
    int x = CenterX2(300+20+160, "", "", false, false);
    int y = CenterY(35);
    m_pSelDirInput = new Fl_File_Input(x, y, 300, 35);
    m_pSelDirInput->value(m_pOwner->GetDestDir());
    
    x += 300 + 20;
    y = CenterY(25);
    m_pSelDirButton = new Fl_Button(x, y, 160, 25, "Select a directory");
    m_pSelDirButton->callback(OpenDirSelWinCB, this);
    
    m_pGroup->end();
    return m_pGroup;
}

void CSelectDirScreen::UpdateLang()
{
    if (m_pDirChooser)
        delete m_pDirChooser;
    
    m_pDirChooser = new Fl_File_Chooser(m_pOwner->GetDestDir(), "*",
                                        (Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE),
                                        GetTranslation("Select destination directory"));
    m_pDirChooser->preview(false);
    m_pDirChooser->previewButton->hide();
    m_pDirChooser->newButton->tooltip(Fl_File_Chooser::new_directory_tooltip);
    
    m_pBox->label(GetTranslation("Select destination directory"));
    m_pSelDirButton->label(GetTranslation("Select a directory"));
}

bool CSelectDirScreen::Next()
{
    m_pOwner->SetDestDir(m_pSelDirInput->value());
    return m_pOwner->VerifyDestDir();
}

void CSelectDirScreen::OpenDirChooser(void)
{
    m_pDirChooser->directory(GetFirstValidDir(m_pSelDirInput->value()).c_str());
    m_pDirChooser->show();
    
    while(m_pDirChooser->visible())
        Fl::wait();

    const char *dir = m_pDirChooser->value();
    if (!dir || !dir[0])
        return;
        
    m_pSelDirInput->value(dir);
}

// -------------------------------------
// Install Files screen
// -------------------------------------

Fl_Group *CInstallFilesScreen::Create()
{
    CBaseScreen::Create();
    
//     m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);

    int x = m_pGroup->x()+20, y = m_pGroup->y()+20;
    m_pProgress = new Fl_Progress(x, y, m_pGroup->w()-(x-m_pGroup->x())-20, 30, "Install progress");
    m_pProgress->minimum(0);
    m_pProgress->maximum(100);
    m_pProgress->value(0);
    
    m_pBuffer = new Fl_Text_Buffer;
    
    y += 50;
    
    m_pDisplay = new Fl_Text_Display(x, y, m_pGroup->w()-(x-m_pGroup->x())-20, m_pGroup->h()-(y-m_pGroup->y())-20, "Status");
    m_pDisplay->buffer(m_pBuffer);
    m_pDisplay->wrap_mode(true, 60);
    
    m_pGroup->end();

    return m_pGroup;
}

void CInstallFilesScreen::UpdateLang()
{
    m_pProgress->label(GetTranslation("Progress"));
    m_pDisplay->label(GetTranslation("Status"));
}

void CInstallFilesScreen::AppendText(const char *txt)
{
    m_pDisplay->insert(txt);
    // Move cursor to last line and last word
    m_pDisplay->scroll(m_pBuffer->length(), m_pDisplay->word_end(m_pBuffer->length()));
    Fl::wait(0.0); // Update screen
}

void CInstallFilesScreen::ChangeStatusText(const char *txt)
{
    m_pDisplay->label(CreateText(txt));
    Fl::wait(0.0); // Update screen
}

// -------------------------------------
// Finish display screen
// -------------------------------------

Fl_Group *CFinishScreen::Create(void)
{
    CBaseScreen::Create();
//     m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    
    m_pBuffer = new Fl_Text_Buffer;

    int x = m_pGroup->x()+20, y = m_pGroup->y()+40;
    m_pDisplay = new Fl_Text_Display(x, y, m_pGroup->w()-(x-m_pGroup->x())-20, m_pGroup->h()-(y-m_pGroup->y())-20,
                                     "Please read the following text");
    m_pDisplay->buffer(m_pBuffer);
    
    m_pGroup->end();
    
    return m_pGroup;
}

void CFinishScreen::UpdateLang()
{
    m_bHasText = (!m_pBuffer->loadfile(m_pOwner->GetLangFinishFName()) ||
            !m_pBuffer->loadfile(m_pOwner->GetFinishFName()));
    m_pDisplay->label(GetTranslation("Please read the following text"));
}

bool CFinishScreen::Activate()
{
    if (!m_bHasText)
        return false;

    m_pOwner->m_pNextButton->label(GetTranslation("Finish"));
    return true;
}

// -------------------------------------
// Lua config screen
// -------------------------------------

void CCFGScreen::SetTitle()
{
    if (!m_szTitle.empty())
    {
        if (m_iLinkedScrMax)
            m_pBoxTitle->label(CreateText("%s (%d/%d)", GetTranslation(m_szTitle.c_str()), m_iLinkedScrNr, m_iLinkedScrMax));
        else
            m_pBoxTitle->label(GetTranslation(m_szTitle.c_str()));
    }
}

Fl_Group *CCFGScreen::Create()
{
    if (m_pGroup)
        return m_pGroup;
    
    CBaseScreen::Create();
    
//     m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);

    m_pBoxTitle = new Fl_Box(CenterX(260), m_pGroup->y()+20, 260, 20, MakeTranslation(m_szTitle));
    
    m_iStartX = m_pGroup->x() + 20;
    m_iStartY = m_pBoxTitle->y() + 20;
    
    m_pGroup->end();
    return m_pGroup;
}

void CCFGScreen::UpdateLang()
{
    for (unsigned u=0; u<m_LuaWidgets.size(); u++)
        m_LuaWidgets[u]->UpdateLanguage();
    
    SetTitle();
}

bool CCFGScreen::Activate()
{
    SetTitle();
    return true;
}

CBaseLuaInputField *CCFGScreen::CreateInputField(const char *label, const char *desc, const char *val, int max, const char *type)
{
    int h = CLuaInputField::CalcHeight(m_pGroup->w() - 80, desc);
    
    if (!m_pNextScreen && ((h + m_iStartY) <= m_pGroup->h()))
    {
        CLuaInputField *field = new CLuaInputField(m_iStartX, m_iStartY, m_pGroup->w() - 80, h, label, desc, val, max, type);
        
        Fl_Group *group = field->Create();
        
        if (group)
        {
            m_pGroup->add(group);
            m_LuaWidgets.push_back(field);
        }
        
        m_iStartY += (h + 10);
        return field;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pOwner->CreateCFGScreen(m_pBoxTitle->label());
    
    return m_pNextScreen->CreateInputField(label, desc, val, max, type);
}

CBaseLuaCheckbox *CCFGScreen::CreateCheckbox(const char *desc, const std::vector<std::string> &l)
{
    int h = CLuaCheckbox::CalcHeight(m_pGroup->w() - 80, desc, l);
    
    if (!m_pNextScreen && ((h + m_iStartY) <= m_pGroup->h()))
    {
        CLuaCheckbox *box = new CLuaCheckbox(m_iStartX, m_iStartY, m_pGroup->w()-80, h, desc, l);
        Fl_Group *group = box->Create();
        
        if (group)
        {
            m_pGroup->add(group);
            m_LuaWidgets.push_back(box);
        }
        
        m_iStartY += (h + 10);
        return box;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pOwner->CreateCFGScreen(m_pBoxTitle->label());
    
    return m_pNextScreen->CreateCheckbox(desc, l);
}

CBaseLuaRadioButton *CCFGScreen::CreateRadioButton(const char *desc, const std::vector<std::string> &l)
{
    int h = CLuaRadioButton::CalcHeight(m_pGroup->w() - 80, desc, l);
    
    if (!m_pNextScreen && ((h + m_iStartY) <= m_pGroup->h()))
    {
        CLuaRadioButton *radio = new CLuaRadioButton(m_iStartX, m_iStartY, m_pGroup->w()-80, h, desc, l);
        Fl_Group *group = radio->Create();
        
        if (group)
        {
            m_pGroup->add(group);
            m_LuaWidgets.push_back(radio);
        }
        
        m_iStartY += (h + 10);
        return radio;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pOwner->CreateCFGScreen(m_pBoxTitle->label());
    
    return m_pNextScreen->CreateRadioButton(desc, l);
}

CBaseLuaDirSelector *CCFGScreen::CreateDirSelector(const char *desc, const char *val)
{
    int h = CLuaDirSelector::CalcHeight(m_pGroup->w() - 80, desc);
    
    if (!m_pNextScreen && ((h + m_iStartY) <= m_pGroup->h()))
    {
        CLuaDirSelector *dir = new CLuaDirSelector(m_iStartX, m_iStartY, m_pGroup->w()-80, h, desc, val);
        Fl_Group *group = dir->Create();
        
        if (group)
        {
            m_pGroup->add(group);
            m_LuaWidgets.push_back(dir);
        }
        
        m_iStartY += (h + 10);
        return dir;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pOwner->CreateCFGScreen(m_pBoxTitle->label());
    
    return m_pNextScreen->CreateDirSelector(desc, val);
}

CBaseLuaCFGMenu *CCFGScreen::CreateCFGMenu(const char *desc)
{
    int h = CLuaCFGMenu::CalcHeight(m_pGroup->w() - 80, desc);
    
    if (!m_pNextScreen && ((h + m_iStartY) <= m_pGroup->h()))
    {
        CLuaCFGMenu *menu = new CLuaCFGMenu(m_iStartX, m_iStartY, m_pGroup->w()-80, h, desc);
        Fl_Group *group = menu->Create();
        
        if (group)
        {
            m_pGroup->add(group);
            m_LuaWidgets.push_back(menu);
        }
        
        m_iStartY += (h + 10);
        return menu;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pOwner->CreateCFGScreen(m_pBoxTitle->label());
    
    return m_pNextScreen->CreateCFGMenu(desc);
}
