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

CInstaller::CInstaller(char **argv) : m_bInstallFiles(false)
{
    // Lazy but handy macro :)
    #define MCreateWidget(w) { widget = new w(this); \
                               group = widget->Create(); \
                               if (group) { m_pWizard->add(group); m_ScreenList.push_back(widget); } }
                               
    if ((InstallInfo.dest_dir_type == DEST_DEFAULT) && !ReadAccess(InstallInfo.dest_dir))
        throwerror(true, CreateText("This installer will install files to the following directory:\n%s\n"
                "However you don't have read permissions to this directory\n"
                        "Please restart the installer as a user who does or as the root user",
                InstallInfo.dest_dir.c_str()));

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

    m_pWizard = new Fl_Wizard(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    
    CBaseScreen *widget;
    Fl_Group *group;
    
    MCreateWidget(CLangScreen);
    MCreateWidget(CWelcomeScreen);
    MCreateWidget(CLicenseScreen);
    
    if (InstallInfo.dest_dir_type == DEST_SELECT)
    {
        MCreateWidget(CSelectDirScreen);
    }
    
    MCreateWidget(CSetParamsScreen);
    MCreateWidget(CInstallFilesScreen);
    MCreateWidget(CFinishScreen);
    
    if (!m_ScreenList.front()->Activate())
        Next();
    
    m_pMainWindow->end();
    Run(argv);
}

CInstaller::~CInstaller()
{
    for(std::list<CBaseScreen *>::iterator p=m_ScreenList.begin();p!=m_ScreenList.end();p++)
        delete *p;
}

void CInstaller::UpdateLanguage(void)
{
    // Update main buttons
    m_pCancelButton->label(GetTranslation("Cancel"));
    m_pPrevButton->label(CreateText("@<-    %s", GetTranslation("Back")));
    m_pNextButton->label(CreateText("%s    @->", GetTranslation("Next")));
    
    // Update all screens
    for(std::list<CBaseScreen *>::iterator p=m_ScreenList.begin();p!=m_ScreenList.end();p++) (*p)->UpdateLang();
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
// Language selection screen
// -------------------------------------

Fl_Group *CLangScreen::Create(void)
{
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();
    
    new Fl_Box((MAIN_WINDOW_W-260)/2, 40, 260, 100, "Please select a language");
    
    m_pChoiceMenu = new Fl_Choice(((MAIN_WINDOW_W-60)/2), (MAIN_WINDOW_H-50)/2, 120, 25,"Language: ");
    m_pChoiceMenu->callback(LangMenuCB);
    
    for (std::list<std::string>::iterator p=InstallInfo.languages.begin();p!=InstallInfo.languages.end();p++)
        m_pChoiceMenu->add(MakeCString(*p));

    m_pChoiceMenu->value(0);
    
    m_pGroup->end();
    
    InstallInfo.cur_lang = InstallInfo.languages.front();
    
    return m_pGroup;
}

bool CLangScreen::Next()
{
    m_pOwner->m_pPrevButton->activate();
    ReadLang();
    m_pOwner->UpdateLanguage();
    return true;
}

bool CLangScreen::Activate()
{
    // Default to first language if there is just one
    return (InstallInfo.languages.size() > 1);
}

// -------------------------------------
// Welcome text display screen
// -------------------------------------

Fl_Group *CWelcomeScreen::Create(void)
{
    Fl::visual(FL_RGB);
    
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();

    m_pImage = Fl_Shared_Image::get(CreateText("%s/%s", InstallInfo.own_dir.c_str(), InstallInfo.intropicname.c_str()));
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
    m_bHasText = (!m_pBuffer->loadfile(CreateText("config/lang/%s/welcome", InstallInfo.cur_lang.c_str())) ||
                  !m_pBuffer->loadfile("config/welcome"));
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
    m_bHasText = (!m_pBuffer->loadfile(CreateText("config/lang/%s/license", InstallInfo.cur_lang.c_str())) ||
                  !m_pBuffer->loadfile("config/license"));
    
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
    m_pSelDirInput->value(InstallInfo.dest_dir.c_str());
    m_pSelDirButton = new Fl_Button((MAIN_WINDOW_W-200), ((MAIN_WINDOW_H-60)-20)/2, 160, 25, "Select a directory");
    m_pSelDirButton->callback(OpenDirSelWinCB, this);
    
    m_pGroup->end();
    return m_pGroup;
}

void CSelectDirScreen::UpdateLang()
{
    if (m_pDirChooser) delete m_pDirChooser;
    m_pDirChooser = new Fl_File_Chooser(InstallInfo.dest_dir.c_str(), "*",
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
    if (!WriteAccess(InstallInfo.dest_dir))
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
        
    InstallInfo.dest_dir = dir;
    m_pSelDirInput->value(dir);
}

// -------------------------------------
// Configuring parameters screen
// -------------------------------------

Fl_Group *CSetParamsScreen::Create()
{
    std::list<command_entry_s *>::iterator p;
    bool empty = true;
    
    for (p=InstallInfo.command_entries.begin();p!=InstallInfo.command_entries.end(); p++)
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
    
    for (p=InstallInfo.command_entries.begin();p!=InstallInfo.command_entries.end(); p++)
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
    for (std::list<command_entry_s*>::iterator p=InstallInfo.command_entries.begin();
         p!=InstallInfo.command_entries.end(); p++)
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
    
    m_pAskPassWindow = new Fl_Window(400, 190, "Password dialog");
    m_pAskPassWindow->set_modal();
    m_pAskPassWindow->begin();
    
    m_pAskPassBox = new Fl_Box(10, 20, 370, 40, "This installation requires root(administrator) privileges in order to "
                                                "continue\nPlease enter the password of the root user");
    m_pAskPassBox->align(FL_ALIGN_WRAP);
    
    m_pAskPassInput = new Fl_Secret_Input(100, 90, 250, 25, "Password: ");
    m_pAskPassInput->take_focus();
    
    m_pAskPassOKButton = new Fl_Return_Button(60, 150, 100, 25, "OK");
    m_pAskPassOKButton->callback(AskPassOKButtonCB, this);
    
    m_pAskPassCancelButton = new Fl_Button(240, 150, 100, 25, "Cancel");
    m_pAskPassCancelButton->callback(AskPassCancelButtonCB, this);
    
    m_pAskPassWindow->end();
    
    m_pGroup->end();

    return m_pGroup;
}

bool CInstallFilesScreen::Activate()
{
    if ((InstallInfo.dest_dir_type == DEST_SELECT) || (InstallInfo.dest_dir_type == DEST_DEFAULT))
    {
        if (!fl_choice(CreateText(GetTranslation("This will install %s to the following directory:\n%s\nContinue?"),
             InstallInfo.program_name.c_str(), MakeCString(InstallInfo.dest_dir)), GetTranslation("Exit program"),
             GetTranslation("Continue"), NULL))
            EndProg();
    }
    else
    {
        if (!fl_choice(CreateText(GetTranslation("This will install %s\nContinue?"), InstallInfo.program_name.c_str()),
             GetTranslation("Exit program"), GetTranslation("Continue"), NULL))
            EndProg();
    }
        
    // Check if we need root access
    bool askpass = false;
    for (std::list<command_entry_s *>::iterator it=InstallInfo.command_entries.begin();
            it!=InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->need_root != NO_ROOT)
        {
            // Command may need root permission, check if it is so
            if ((*it)->need_root == DEPENDED_ROOT)
            {
                param_entry_s *p = GetParamByVar((*it)->dep_param);
                if (p && !WriteAccess(p->value))
                {
                    (*it)->need_root = NEED_ROOT;
                    if (!askpass) askpass = true;
                }
            }
            else if (!askpass) askpass = true;
        }
    }

    if (!askpass)
        askpass = !WriteAccess(InstallInfo.dest_dir);

    m_SUHandler.SetOutputFunc(SUOutputHandler, this);
    m_SUHandler.SetUser("root");
    m_SUHandler.SetTerminalOutput(false);

    if (askpass && m_SUHandler.NeedPassword())
    {
        while(true)
        {
            CleanPasswdString(m_szPassword);
            m_szPassword = NULL;

            m_pAskPassWindow->hotspot(m_pAskPassOKButton);
            m_pAskPassWindow->take_focus();
            m_pAskPassWindow->show();

            while(m_pAskPassWindow->visible()) Fl::wait();

            // Check if password is invalid
            if (!m_szPassword)
            {
                if (fl_choice(GetTranslation("Root access is required to continue\nAbort installation?"),
                    GetTranslation("No"), GetTranslation("Yes"), NULL))
                    EndProg();
            }
            else
            {
                if (m_SUHandler.TestSU(m_szPassword))
                    break;

                // Some error appeared
                if (m_SUHandler.GetError() == LIBSU::CLibSU::SU_ERROR_INCORRECTPASS)
                    fl_alert(GetTranslation("Incorrect password given for root user\nPlease retype"));
                else
                {
                    throwerror(true, GetTranslation("Could not use su to gain root access"
                               "Make sure you can use su(adding the current user to the wheel group may help"));
                }
            }
        }
    }
    
    if (chdir(InstallInfo.dest_dir.c_str()))
        throwerror(true, "Could not open directory '%s'", InstallInfo.dest_dir.c_str());
    
    m_pOwner->m_bInstallFiles = true;
    m_pOwner->m_pPrevButton->deactivate();
    m_pOwner->m_pNextButton->deactivate();
    
    Install();
    return true;
}

void CInstallFilesScreen::UpdateLang()
{
    m_pProgress->label(GetTranslation("Progress"));
    m_pDisplay->label(GetTranslation("Status"));
    m_pAskPassWindow->label(GetTranslation("Password dialog"));
    m_pAskPassInput->label(CreateText("%s: ", GetTranslation("Password")));
    m_pAskPassBox->label(GetTranslation("This installation requires root(administrator) privileges in order to continue\n"
            "Please enter the password of the root user"));
    m_pAskPassOKButton->label(GetTranslation("OK"));
    m_pAskPassCancelButton->label(GetTranslation("Cancel"));
}

void CInstallFilesScreen::AppendText(const char *txt)
{
    m_pDisplay->insert(txt);
    // Move cursor to last line and last word
    m_pDisplay->scroll(m_pBuffer->length(), m_pDisplay->word_end(m_pBuffer->length()));
}

void CInstallFilesScreen::SetPassword(bool unset)
{
    CleanPasswdString(m_szPassword);
    m_szPassword = NULL;
    
    if (!unset)
    {
        const char *passwd = m_pAskPassInput->value();
    
        if (passwd && passwd[0])
        {
            m_szPassword = strdup(passwd);

            // Can't use FLTK's replace() to delete input field text, since it stores undo info
            int length = strlen(passwd);
            
            char str0[length];
            for (int i=0;i<length;i++) str0[i] = 0;
            m_pAskPassInput->value(str0);
            
            // Force clean temp inputfield string
            char *str = const_cast<char *>(passwd);
            for(int i=0;i<strlen(str);i++) str[i] = 0;
        }
    }
    m_pAskPassWindow->hide();
    m_pAskPassInput->value(NULL);
}

void CInstallFilesScreen::SUUpdateProgress(int percent, void *p)
{
    ((CInstallFilesScreen *)p)->SetProgress(percent/(1+InstallInfo.command_entries.size()));
    Fl::wait(0.0); // Update screen
}

void CInstallFilesScreen::SUUpdateText(const std::string &str, void *p)
{
    ((CInstallFilesScreen *)p)->AppendText(str);
    Fl::wait(0.0); // Update screen
}

void CInstallFilesScreen::AskPassOKButtonCB(Fl_Widget *w, void *p)
{
    ((CInstallFilesScreen *)p)->SetPassword(false);
}

void CInstallFilesScreen::AskPassCancelButtonCB(Fl_Widget *w, void *p)
{
    ((CInstallFilesScreen *)p)->SetPassword(true);
}

void CInstallFilesScreen::ChangeStatusText(const char *txt, int n)
{
    // Install entries + 1 because it doesn't include extraction
    m_pDisplay->label(CreateText("%s: %s (%d/%d)", GetTranslation("Status"), GetTranslation(txt), n,
                      InstallInfo.command_entries.size()+1));
}

void CInstallFilesScreen::Install()
{
    std::string curfile;
    short percent = 0;
    
    ChangeStatusText("Extracting files", 1);

    bool alwaysroot = false;
    if (!WriteAccess(InstallInfo.dest_dir))
    {
        CExtractAsRootFunctor Extracter;

        Extracter.SetUpdateProgFunc(SUUpdateProgress, this);
        Extracter.SetUpdateTextFunc(SUUpdateText, this);
        
        if (!Extracter(m_szPassword))
        {
            CleanPasswdString(m_szPassword);
            m_szPassword = NULL;
            throwerror(true, "Error during extracting files");
        }

        AppendText("Done!\n");
        alwaysroot = true; // Install commands need root now too
    }
    else
    {
        while(1)
        {
            percent = ExtractArchive(curfile);
            AppendText("Extracting file: " + curfile + "\n");
    
            if (percent == -1)
            {
                CleanPasswdString(m_szPassword);
                m_szPassword = NULL;
                throwerror(true, "Error during extracting files");
            }
    
            if (percent == 100)
            {
                percent = 100/(1+InstallInfo.command_entries.size()); // Convert to overall progress
                AppendText("Done!\n");
                break;
            }
        
            SetProgress(percent/(1+InstallInfo.command_entries.size()));
            Fl::wait(0.0); // Update screen
        }
    }
    
    short step = 2; // Not 1, because extracting files is also a step
    for (std::list<command_entry_s*>::iterator it=InstallInfo.command_entries.begin();
         it!=InstallInfo.command_entries.end(); it++, step++)
    {
        if ((*it)->command.empty()) continue;
        
        std::string command = (*it)->command + " " + GetParameters(*it);
    
        AppendText(CreateText("\nExecute: %s\n\n", command.c_str()));
        ChangeStatusText((*it)->description.c_str(), step);

        if ((*it)->need_root == NEED_ROOT || alwaysroot)
        {
            m_SUHandler.SetPath((*it)->path.c_str());
            m_SUHandler.SetCommand(command);
            if (!m_SUHandler.ExecuteCommand(m_szPassword))
            {
                if ((*it)->exit_on_failure)
                {
                    CleanPasswdString(m_szPassword);
                    m_szPassword = NULL;
                    throwerror(true, "%s\n('%s')", GetTranslation("Failed to execute install command"),
                               m_SUHandler.GetErrorMsgC());
                }
            }
        }
        else
        {
            // Redirect stderr to stdout, so that errors will be displayed too
            command += " 2>&1";
            
            setenv("PATH", (*it)->path.c_str(), 1);
            FILE *pPipe = popen(command.c_str(), "r");
            if (pPipe)
            {
                char buf[1024];
                while(fgets(buf, sizeof(buf), pPipe))
                {
                    AppendText(buf);
                    Fl::wait(0.0); // Update screen
                }
                
                // Check if command exitted normally and close pipe
                int state = pclose(pPipe);
                if (!WIFEXITED(state) || (WEXITSTATUS(state) == 127)) // SH returns 127 if command execution failes
                {
                    if ((*it)->exit_on_failure)
                    {
                        CleanPasswdString(m_szPassword);
                        m_szPassword = NULL;
                        throwerror(true, "Failed to execute install command");
                    }
                }
            }
            else
            {
                CleanPasswdString(m_szPassword);
                m_szPassword = NULL;
                throwerror(true, "Could not execute installation commands (could not open pipe)");
            }
        }
        
        percent += (1.0f/((float)InstallInfo.command_entries.size()+1.0f))*100.0f;
        SetProgress(percent);
    }

    AppendText("Registering installation...");
    Register.RegisterInstall();
    AppendText("done\n");

    SetProgress(100);
    m_pOwner->m_bInstallFiles = false;
    fl_message(GetTranslation("Installation of %s complete!"), InstallInfo.program_name.c_str());
    CleanPasswdString(m_szPassword);
    m_szPassword = NULL;
    
    m_pOwner->m_pCancelButton->deactivate();
    m_pOwner->m_pPrevButton->deactivate();
    m_pOwner->m_pNextButton->activate();

    // HACK
    if (!FileExists(InstallInfo.own_dir + "/config/finish"))
        m_pOwner->m_pNextButton->label(GetTranslation("Finish"));
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
    m_bHasText = (!m_pBuffer->loadfile(CreateText("%s/config/lang/%s/finish", InstallInfo.own_dir.c_str(),
                   InstallInfo.cur_lang.c_str())) || !m_pBuffer->loadfile(CreateText("%s/config/finish",
                   InstallInfo.own_dir.c_str())));
    m_pDisplay->label(GetTranslation("Please read the following text"));
}

bool CFinishScreen::Activate()
{
    if (!m_bHasText)
        return false;

    m_pOwner->m_pNextButton->label(GetTranslation("Finish"));
    return true;
}
