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
    // Lazy but handy macro :)
    #define MCreateWidget(w) { widget = new w(this); \
                               group = widget->Create(); \
                               if (group) { m_pWizard->add(group); m_ScreenList.push_back(widget); } }
                               
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
    
    m_pWizard = new Fl_Wizard(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
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
    
/*    CBaseScreen *widget;
    
    MCreateWidget(CLangScreen);
    MCreateWidget(CWelcomeScreen);
    MCreateWidget(CLicenseScreen);
    
    if (m_InstallInfo.dest_dir_type == DEST_SELECT)
    {
        MCreateWidget(CSelectDirScreen);
    }
    
    MCreateWidget(CSetParamsScreen);
    
    //MCreateWidget(CInstallFilesScreen);
    m_pInstallFilesScreen = new CInstallFilesScreen(this);
    m_pWizard->add(m_pInstallFilesScreen->Create());
    m_ScreenList.push_back(m_pInstallFilesScreen);
    
    MCreateWidget(CFinishScreen);*/
    
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
    
    if (!m_pBox)
    {
        m_pBox = new Fl_Box(m_pGroup->x(), m_pGroup->y(), m_iWidth, TitleHeight(m_iWidth, m_szDescription.c_str()), MakeCString(m_szDescription));
        m_pBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_WRAP);
    }
    else
        m_pBox->label(MakeCString(m_szDescription));
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
                               const char *val, int max) : CBaseLuaWidget(x, y, w, h, desc), m_iMax(max)
{
    if (label && *label)
        m_szLabel = label;
    
    if (val && *val)
        m_szValue = val;
}

Fl_Group *CLuaInputField::Create()
{
    Fl_Group *group = CBaseLuaWidget::Create();
    
    group->add(m_pInput = new Fl_Input(group->x(), group->y() + DescHeight(), group->w(), m_iFieldHeight, MakeCString(GetTranslation(m_szLabel))));
    m_pInput->align(FL_ALIGN_LEFT);
    
    if (!m_szValue.empty())
        m_pInput->value(m_szValue.c_str());
    
    return group;
}

void CLuaInputField::UpdateLanguage()
{
    CBaseLuaWidget::UpdateLanguage();
    
    m_pInput->label(MakeCString(GetTranslation(m_szLabel)));
}

int CLuaInputField::CalcHeight(int w, const char *desc)
{
    return m_iFieldHeight + TitleHeight(w, desc);
}

// -------------------------------------
// Lua checkbox class
// -------------------------------------

int CLuaCheckbox::m_iButtonHeight = 20;
int CLuaCheckbox::m_iButtonSpace = 10;

CLuaCheckbox::CLuaCheckbox(int x, int y, int w, int h, const char *desc,
                           const std::vector<std::string> &l) : CBaseLuaWidget(x, y, w, h, desc), m_Options(l)
{
}

Fl_Group *CLuaCheckbox::Create()
{
    Fl_Group *group = CBaseLuaWidget::Create();
    int y = group->y() + DescHeight(), w = 0;
    
    // Find longest text
    for (std::vector<std::string>::const_iterator it=m_Options.begin(); it!=m_Options.end(); it++)
        w = Max(w, fl_width(it->c_str()));

    for (std::vector<std::string>::const_iterator it=m_Options.begin(); it!=m_Options.end(); it++)
    {
        Fl_Check_Button *button = new Fl_Check_Button(group->x(), y, 60, m_iButtonHeight, MakeCString(*it));
        button->type(FL_TOGGLE_BUTTON);
        m_Buttons.push_back(button);
        group->add(button);
        y += (m_iButtonHeight + m_iButtonSpace);
    }
    
    return group;
}

void CLuaCheckbox::UpdateLanguage()
{
    CBaseLuaWidget::UpdateLanguage(); /* UNDONE */
}

int CLuaCheckbox::CalcHeight(int w, const char *desc, const std::vector<std::string> &l)
{
    if (!l.empty())
        return (m_iButtonHeight * l.size()) + (m_iButtonSpace * (l.size()-1)) + TitleHeight(w, desc);
    else
        return TitleHeight(w, desc);
}

// -------------------------------------
// Lua checkbox class
// -------------------------------------

int CLuaRadioButton::m_iButtonHeight = 20;
int CLuaRadioButton::m_iButtonSpace = 10;

CLuaRadioButton::CLuaRadioButton(int x, int y, int w, int h, const char *desc,
                                 const std::vector<std::string> &l) : CBaseLuaWidget(x, y, w, h, desc), m_Options(l)
{
}

Fl_Group *CLuaRadioButton::Create()
{
    Fl_Group *group = CBaseLuaWidget::Create();
    int y = group->y() + DescHeight(), w = 0;
    
    // Find longest text
    for (std::vector<std::string>::const_iterator it=m_Options.begin(); it!=m_Options.end(); it++)
        w = Max(w, fl_width(it->c_str()));

    for (std::vector<std::string>::const_iterator it=m_Options.begin(); it!=m_Options.end(); it++)
    {
        Fl_Round_Button *button = new Fl_Round_Button(group->x(), y, 60, m_iButtonHeight, MakeCString(*it));
        button->type(FL_RADIO_BUTTON);
        m_Buttons.push_back(button);
        group->add(button);
        y += (m_iButtonHeight + m_iButtonSpace);
    }
    
    return group;
}

void CLuaRadioButton::UpdateLanguage()
{
    CBaseLuaWidget::UpdateLanguage(); /* UNDONE */
}

int CLuaRadioButton::CalcHeight(int w, const char *desc, const std::vector<std::string> &l)
{
    if (!l.empty())
        return (m_iButtonHeight * l.size()) + (m_iButtonSpace * (l.size()-1)) + TitleHeight(w, desc);
    else
        return TitleHeight(w, desc);
}

// -------------------------------------
// Language selection screen
// -------------------------------------

Fl_Group *CLangScreen::Create(void)
{
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();
    
    new Fl_Box((MAIN_WINDOW_W-260)/2, 40, 260, 100, "Please select a language");
    
    m_pChoiceMenu = new Fl_Choice(((MAIN_WINDOW_W-60)/2), (MAIN_WINDOW_H-50)/2, 120, 25,"Language: ");
    m_pChoiceMenu->callback(LangMenuCB, this);
    
    for (std::list<std::string>::iterator p=m_pOwner->m_Languages.begin();
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
    // Default to first language if there is just one
    return (m_pOwner->m_Languages.size() > 1);
}

// -------------------------------------
// Welcome text display screen
// -------------------------------------

Fl_Group *CWelcomeScreen::Create(void)
{
    Fl::visual(FL_RGB);
    
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();

    m_pImage = Fl_Shared_Image::get(m_pOwner->GetIntroPicFName());
    if (m_pImage)
    {
        m_pImageBox = new Fl_Box(40, 60, 165, (MAIN_WINDOW_H-120));

        // Scale image if its to big
        if (m_pImage->w() > m_pImageBox->w() || m_pImage->h() > m_pImageBox->h())
        {
            int w, h;
            float wfact, hfact;

            // Check which scale factor is the biggest and use that one. This makes sure that the image will fit.
            wfact = (float(m_pImage->w())/float(m_pImageBox->w()));
            hfact = (float(m_pImage->h())/float(m_pImageBox->h()));

            if (wfact >= hfact)
            {
                w = m_pImageBox->w();
                h = (int)(float(m_pImage->h()) / wfact);
            }
            else
            {
                w = (int)(float(m_pImage->w()) / hfact);
                h = m_pImageBox->h();
            }

            Fl_Image *temp = m_pImage->copy(w, h);
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

        m_pDisplay = new Fl_Text_Display(m_pImageBox->w() + 60, 60, (MAIN_WINDOW_W-20-m_pImageBox->w()-60),
                                         (MAIN_WINDOW_H-120), "Welcome");
    }
    else
        m_pDisplay = new Fl_Text_Display(60, 60, (MAIN_WINDOW_W-90), (MAIN_WINDOW_H-120), "Welcome");
    
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
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();
    
    m_pBuffer = new Fl_Text_Buffer;
    
    m_pDisplay = new Fl_Text_Display(60, 60, (MAIN_WINDOW_W-90), (MAIN_WINDOW_H-160), "License agreement");
    m_pDisplay->buffer(m_pBuffer);
    
    m_pCheckButton = new Fl_Check_Button((MAIN_WINDOW_W-350)/2, (MAIN_WINDOW_H-80), 350, 25,
                                         "I Agree to this license agreement");
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
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();

    m_pBox = new Fl_Box((MAIN_WINDOW_W-260)/2, 40, 260, 100, "Select destination directory");
    m_pSelDirInput = new Fl_Output(80, ((MAIN_WINDOW_H-60)-20)/2, 300, 25, "dir: ");
    m_pSelDirInput->value(m_pOwner->m_szDestDir.c_str());
    m_pSelDirButton = new Fl_Button((MAIN_WINDOW_W-200), ((MAIN_WINDOW_H-60)-20)/2, 160, 25, "Select a directory");
    m_pSelDirButton->callback(OpenDirSelWinCB, this);
    
    m_pGroup->end();
    return m_pGroup;
}

void CSelectDirScreen::UpdateLang()
{
    if (m_pDirChooser) delete m_pDirChooser;
    m_pDirChooser = new Fl_File_Chooser(m_pOwner->m_szDestDir.c_str(), "*",
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
    if (!WriteAccess(m_pOwner->m_szDestDir))
    {
        return (fl_choice(GetTranslation("You don't have write permissions for this directory.\n"
                "The files can be extracted as the root user,\n"
                "but you'll need to enter the root password for this later."),
                GetTranslation("Choose another directory"),
                GetTranslation("Continue as root"), NULL) == 1);
    }
    return true;
}

void CSelectDirScreen::OpenDirChooser(void)
{
    m_pDirChooser->show();
    while(m_pDirChooser->visible()) Fl::wait();

    char *dir = const_cast<char *>(m_pDirChooser->value());
    if (!dir || !dir[0])
        return;
        
    m_pOwner->m_szDestDir = dir;
    m_pSelDirInput->value(dir);
}

// -------------------------------------
// Configuring parameters screen
// -------------------------------------

Fl_Group *CSetParamsScreen::Create()
{
    std::list<command_entry_s *>::iterator p;
    bool empty = true;
    
    for (p=m_pOwner->m_InstallInfo.command_entries.begin();p!=m_pOwner->m_InstallInfo.command_entries.end(); p++)
    {
        if (!(*p)->parameter_entries.empty()) { empty = false; break; }
    }
    
    if (empty) return NULL;
    
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();

    m_pBoxTitle = new Fl_Box((MAIN_WINDOW_W-260)/2, 20, 260, 100, "Configure parameters");
    
    const int iChoiceW = 250, iDescW = 250;
    int x = 40, y = 120;

    m_pChoiceBrowser = new Fl_Hold_Browser(x, y, iChoiceW, 145, "Parameters");
    
    for (p=m_pOwner->m_InstallInfo.command_entries.begin();p!=m_pOwner->m_InstallInfo.command_entries.end(); p++)
    {
        for (std::map<std::string, param_entry_s *>::iterator p2=(*p)->parameter_entries.begin();
             p2!=(*p)->parameter_entries.end();p2++)
            m_pChoiceBrowser->add(p2->first.c_str(), *p);
    }
    
    m_pChoiceBrowser->callback(ParamBrowserCB, this);
    m_pChoiceBrowser->align(FL_ALIGN_TOP);
    
    x += (iChoiceW + 20);
    m_pDescriptionOutput = new Fl_Multiline_Output(x, y, iDescW, 100, "Description");
    m_pDescriptionOutput->align(FL_ALIGN_TOP);
    m_pDescriptionOutput->wrap(1);

    y += 120;
    m_pDefOutput = new Fl_Output(x, y, iDescW, 25, "Default");
    m_pDefOutput->align(FL_ALIGN_TOP);
    
    x=100;
    y+=40;

    // List of parameter options
    m_pParamInput = new Fl_Input(x, y, 250, 25, "Value: ");
    m_pParamInput->callback(ParamInputCB, this);
    m_pParamInput->when(FL_WHEN_CHANGED);

    // Input field for a parameter
    m_pValChoiceMenu = new Fl_Choice(x, y, 150, 25,"Value: ");
    m_pValChoiceMenu->callback(ValChoiceMenuCB, this);

    // Dir selecter for parameter
    m_pSelDirInput = new Fl_Output(x, y, 250, 25, "Value: ");
    m_pSelDirButton = new Fl_Button(x + 275, y, 120, 25, "Change");
    m_pSelDirButton->callback(OpenDirSelWinCB, this);
    
    m_pGroup->end();
    return m_pGroup;
}

void CSetParamsScreen::UpdateLang()
{
    m_pBoxTitle->label(GetTranslation("Configure parameters"));
    m_pChoiceBrowser->label(GetTranslation("Parameters"));
    m_pDescriptionOutput->label(GetTranslation("Description"));
    m_pDefOutput->label(GetTranslation("Default"));
    m_pParamInput->label(CreateText("%s: ", GetTranslation("Value")));
    m_pValChoiceMenu->label(CreateText("%s: ", GetTranslation("Value")));
    m_pSelDirInput->label(CreateText("%s: ", GetTranslation("Value")));
    m_pSelDirButton->label(GetTranslation("Change"));
    
    // Create dir selecter (need to do this when the language changes!)
    if (m_pDirChooser) delete m_pDirChooser;
    m_pDirChooser = new Fl_File_Chooser("~", "*",
                                        (Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE),
                                        GetTranslation("Select a directory"));
    m_pDirChooser->preview(false);
    m_pDirChooser->previewButton->hide();
    m_pDirChooser->newButton->tooltip(Fl_File_Chooser::new_directory_tooltip);
}

bool CSetParamsScreen::Activate()
{
    // Get first param entry
    for (std::list<command_entry_s*>::iterator p=m_pOwner->m_InstallInfo.command_entries.begin();
         p!=m_pOwner->m_InstallInfo.command_entries.end(); p++)
    {
        if (!(*p)->parameter_entries.empty())
        {
            SetInput((*p)->parameter_entries.begin()->first.c_str(), *p);
            break;
        }
    }
    
    m_pChoiceBrowser->value(1);
    
    return true;
};

void CSetParamsScreen::SetInput(const char *txt, command_entry_s *pCommandEntry)
{
    m_pCurrentParamEntry = pCommandEntry->parameter_entries[txt];
    if (m_pCurrentParamEntry->param_type == PTYPE_STRING)
    {
        m_pValChoiceMenu->hide();
        m_pSelDirButton->hide();
        m_pSelDirInput->hide();
        m_pParamInput->show();
        m_pParamInput->value(m_pCurrentParamEntry->value.c_str());
    }
    else if (m_pCurrentParamEntry->param_type == PTYPE_DIR)
    {
        m_pValChoiceMenu->hide();
        m_pParamInput->hide();
        m_pSelDirButton->show();
        m_pSelDirInput->show();
        m_pSelDirInput->value(m_pCurrentParamEntry->value.c_str());
    }
    else
    {
        short s=0;
        m_pValChoiceMenu->clear();
        
        if (m_pCurrentParamEntry->param_type == PTYPE_BOOL)
        {
            m_pValChoiceMenu->add(GetTranslation("Enable"));
            m_pValChoiceMenu->add(GetTranslation("Disable"));
            if (m_pCurrentParamEntry->value == "false") m_pValChoiceMenu->value(1);
            else m_pValChoiceMenu->value(0);
        }
        else
        {
            for (std::list<std::string>::iterator p=m_pCurrentParamEntry->options.begin();
                 p!=m_pCurrentParamEntry->options.end();p++,s++)
            {
                m_pValChoiceMenu->add(MakeCString(*p));
                if (*p == m_pCurrentParamEntry->value) { debugline("val: %s", p->c_str()); m_pValChoiceMenu->value(s); }
            }
        }
        m_pParamInput->hide();
        m_pSelDirButton->hide();
        m_pSelDirInput->hide();
        m_pValChoiceMenu->show();
    }
    
    m_pDescriptionOutput->value(GetTranslation(m_pCurrentParamEntry->description.c_str()));
    
    const char *str = m_pCurrentParamEntry->defaultval.c_str();
    if (m_pCurrentParamEntry->param_type == PTYPE_BOOL)
    {
        if (m_pCurrentParamEntry->defaultval == "true") str = GetTranslation("Enabled");
        else str = GetTranslation("Disabled");
    }
    m_pDefOutput->value(str);
}

void CSetParamsScreen::SetValue(const std::string &str)
{
    if (m_pCurrentParamEntry->param_type == PTYPE_BOOL)
    {
        if (str == GetTranslation("Enable")) m_pCurrentParamEntry->value = "true";
        else m_pCurrentParamEntry->value = "false";
    }
    else
        m_pCurrentParamEntry->value = str;
}

void CSetParamsScreen::OpenDirChooser(void)
{
    m_pDirChooser->directory(GetFirstValidDir(m_pCurrentParamEntry->value).c_str());
    m_pDirChooser->show();
    while(m_pDirChooser->visible()) Fl::wait();
    
    if (m_pDirChooser->value())
    {
        SetValue(m_pDirChooser->value());
        m_pSelDirInput->value(m_pDirChooser->value());
    }
}

void CSetParamsScreen::ParamBrowserCB(Fl_Widget *w, void *p)
{
    std::map<std::string, param_entry_s *>::iterator it;
    short s=0, value=((Fl_Hold_Browser*)w)->value();
    command_entry_s *pCommandEntry = ((command_entry_s *)((Fl_Hold_Browser*)w)->data(value));

    for (it=pCommandEntry->parameter_entries.begin(); s<(value-1); it++, s++);
    
    ((CSetParamsScreen *)p)->SetInput(it->first.c_str(), pCommandEntry);
}

void CSetParamsScreen::ValChoiceMenuCB(Fl_Widget *w, void *p)
{
    ((CSetParamsScreen *)p)->SetValue(((Fl_Menu_*)w)->mvalue()->text);
}

void CSetParamsScreen::ParamInputCB(Fl_Widget *w, void *p)
{
    ((CSetParamsScreen *)p)->SetValue(((Fl_Input*)w)->value());
}

// -------------------------------------
// Base Install screen
// -------------------------------------

Fl_Group *CInstallFilesScreen::Create()
{
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();

    m_pProgress = new Fl_Progress(50, 60, (MAIN_WINDOW_W-100), 30, "Install progress");
    m_pProgress->minimum(0);
    m_pProgress->maximum(100);
    m_pProgress->value(0);
    
    m_pBuffer = new Fl_Text_Buffer;
    
    m_pDisplay = new Fl_Text_Display(50, 110, (MAIN_WINDOW_W-100), (MAIN_WINDOW_H-170), "Status");
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
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();
    
    m_pBuffer = new Fl_Text_Buffer;

    m_pDisplay = new Fl_Text_Display(60, 60, (MAIN_WINDOW_W-90), (MAIN_WINDOW_H-120), "Please read the following text");
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

Fl_Group *CCFGScreen::Create()
{
    if (m_pGroup)
        return m_pGroup;
    
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();

    m_iStartY = 60;
    
    m_pBoxTitle = new Fl_Box((MAIN_WINDOW_W-260)/2, 20, 260, 20, "UNDONE");
    
    m_pGroup->end();
    return m_pGroup;
}

CBaseLuaInputField *CCFGScreen::CreateInputField(const char *label, const char *desc, const char *val, int max)
{
    int h = CLuaInputField::CalcHeight(m_pGroup->w() - 80, desc);
    
    if (!m_pNextScreen && ((h + m_iStartY) <= m_pGroup->h()))
    {
        CLuaInputField *field = new CLuaInputField(60, m_iStartY, m_pGroup->w() - 80, h, label, desc, val, max);
        Fl_Group *group = field->Create();
        if (group)
            m_pGroup->add(group);
        m_iStartY += (h + 10);
        return field;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pOwner->CreateCFGScreen(m_pBoxTitle->label());
    
    return m_pNextScreen->CreateInputField(label, desc, val, max);
}

CBaseLuaCheckbox *CCFGScreen::CreateCheckbox(const char *desc, const std::vector<std::string> &l)
{
    int h = CLuaCheckbox::CalcHeight(m_pGroup->w() - 80, desc, l);
    
    if (!m_pNextScreen && ((h + m_iStartY) <= m_pGroup->h()))
    {
        CLuaCheckbox *box = new CLuaCheckbox(60, m_iStartY, m_pGroup->w()-80, h, desc, l);
        Fl_Group *group = box->Create();
        if (group)
            m_pGroup->add(group);
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
        CLuaRadioButton *radio = new CLuaRadioButton(60, m_iStartY, m_pGroup->w()-80, h, desc, l);
        Fl_Group *group = radio->Create();
        if (group)
            m_pGroup->add(group);
        m_iStartY += (h + 10);
        return radio;
    }
    
    if (!m_pNextScreen)
        m_pNextScreen = (CCFGScreen *)m_pOwner->CreateCFGScreen(m_pBoxTitle->label());
    
    return m_pNextScreen->CreateRadioButton(desc, l);
}
