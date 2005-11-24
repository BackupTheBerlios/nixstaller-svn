#include "fltk.h"

// -------------------------------------
// Language selection screen
// -------------------------------------

Fl_Group *CLangScreen::Create(void)
{
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();
    
    m_pMenuItems = new Fl_Menu_Item[InstallInfo.languages.size()+1];
    
    new Fl_Box((MAIN_WINDOW_W-260)/2, 40, 260, 100, "Please select a language");
    
    short s=0;
    for (std::list<char*>::iterator p=InstallInfo.languages.begin();p!=InstallInfo.languages.end();p++, s++)
        m_pMenuItems[s].text = *p;
        
    m_pMenuItems[s].text = NULL;
    m_pChoiceMenu = new Fl_Choice(((MAIN_WINDOW_W-60)/2), (MAIN_WINDOW_H-50)/2, 120, 25,"&Language: ");
    m_pChoiceMenu->menu(m_pMenuItems);
    m_pChoiceMenu->callback(LangMenuCB);
    
    m_pGroup->end();
    
    return m_pGroup;
}

bool CLangScreen::Next()
{
    pPrevButton->activate();
    ReadLang();
    UpdateLanguage();
    return true;
}

bool CLangScreen::Activate()
{
    pPrevButton->deactivate();
    
    // Default to first language if there is just one
    if (InstallInfo.languages.size() == 1)
    {
        // HACK
        extern void WizNextCB(Fl_Widget *, void *);
        WizNextCB(NULL, NULL);
    }
    
    return true;
}

// -------------------------------------
// Welcome text display screen
// -------------------------------------

Fl_Group *CWelcomeScreen::Create(void)
{
    m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    m_pGroup->begin();
    
    m_pBuffer = new Fl_Text_Buffer;

    m_pDisplay = new Fl_Text_Display(60, 60, (MAIN_WINDOW_W-90), (MAIN_WINDOW_H-120), "Welcome");
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
    
    m_pDisplay = new Fl_Text_Display(60, 60, (MAIN_WINDOW_W-90), (MAIN_WINDOW_H-160), "License Agreement");
    m_pDisplay->buffer(m_pBuffer);
    
    m_pCheckButton = new Fl_Check_Button((MAIN_WINDOW_W-350)/2, (MAIN_WINDOW_H-80), 350, 25,
                                         "I Agree to this license agreement");
    m_pCheckButton->callback(LicenseCheckCB);
    
    m_pGroup->end();
    return m_pGroup;
}

void CLicenseScreen::UpdateLang()
{
    m_bHasText = (!m_pBuffer->loadfile(CreateText("config/lang/%s/license", InstallInfo.cur_lang.c_str())) ||
                  !m_pBuffer->loadfile("config/license"));
    
    m_pDisplay->label(GetTranslation("License Agreement"));
    m_pCheckButton->label(GetTranslation("I Agree to this license agreement"));
}

bool CLicenseScreen::Activate()
{
    if (!m_bHasText) return false;
    
    if (!m_pCheckButton->value()) pNextButton->deactivate();
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
    m_pSelDirButton = new Fl_Button((MAIN_WINDOW_W-200), ((MAIN_WINDOW_H-60)-20)/2, 160, 25, "Select directory");
    m_pSelDirButton->callback(OpenDirSelWinCB, this);
    
    m_pGroup->end();
    return m_pGroup;
}

void CSelectDirScreen::UpdateLang()
{
    if (m_pDirChooser) delete m_pDirChooser;
    m_pDirChooser = new Fl_File_Chooser(InstallInfo.dest_dir.c_str(), "*",
                                        (Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE),
                                        "Select destination directory");
    m_pDirChooser->preview(false);
    m_pDirChooser->previewButton->hide();

    m_pDirChooser->label(GetTranslation("Select destination directory"));
    m_pDirChooser->newButton->tooltip(Fl_File_Chooser::new_directory_tooltip);
    m_pBox->label(GetTranslation("Select destination directory"));
    m_pSelDirButton->label(GetTranslation("Select directory"));
}

bool CSelectDirScreen::Next()
{
    char temp[128];
    sprintf(temp, GetTranslation("This will install %s to the following directory:"), InstallInfo.program_name);
    return (fl_ask("%s\n%s\n%s", temp, InstallInfo.dest_dir.c_str(), GetTranslation("Continue?")));
}

void CSelectDirScreen::OpenDirChooser(void)
{
    m_pDirChooser->show();
    while(m_pDirChooser->visible()) Fl::wait();
    
    if (m_pDirChooser->value())
    {
        InstallInfo.dest_dir = m_pDirChooser->value();
        m_pSelDirInput->value(InstallInfo.dest_dir.c_str());
    }
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

    m_pBoxTitle = new Fl_Box((MAIN_WINDOW_W-260)/2, 20, 260, 100, "Configuring parameters");
    
    const int iChoiceW = 250, iDescW = 250, iTotalW = iChoiceW + 10 + iDescW;
    int x = 40, y = ((MAIN_WINDOW_H-50)/2)-40;

    m_pChoiceBrowser = new Fl_Hold_Browser(x, y, iChoiceW, 100, "Parameters");
    
    for (p=InstallInfo.command_entries.begin();p!=InstallInfo.command_entries.end(); p++)
    {
        for (std::map<std::string, param_entry_s *>::iterator p2=(*p)->parameter_entries.begin();
             p2!=(*p)->parameter_entries.end();p2++)
            m_pChoiceBrowser->add(p2->first.c_str(), *p);
    }
    
    m_pChoiceBrowser->callback(ParamBrowserCB, this);
    m_pChoiceBrowser->align(FL_ALIGN_TOP);
    
    x += (iChoiceW + 20);
    m_pDescriptionOutput = new Fl_Multiline_Output(x, y, iDescW, 75, "Description");
    m_pDescriptionOutput->align(FL_ALIGN_TOP);
    m_pDescriptionOutput->wrap(1);
    
    x=100;
    y+=120;
    m_pParamInput = new Fl_Input(x, y, 250, 25, "Value: ");
    m_pParamInput->callback(ParamInputCB, this);
    m_pParamInput->when(FL_WHEN_CHANGED);
    
    m_pValChoiceMenu = new Fl_Choice(x, y, 150, 25,"Value: ");
    m_pValChoiceMenu->callback(ValChoiceMenuCB, this);

    x-=50;
    y+=25;
    m_pDefaultValBox = new Fl_Box(x, y, 250, 25, "Default: ");
    
    m_pGroup->end();
    return m_pGroup;
}

void CSetParamsScreen::UpdateLang()
{
    m_pBoxTitle->label(GetTranslation("Configuring parameters"));
    m_pChoiceBrowser->label(GetTranslation("Parameters"));
    m_pParamInput->label(CreateText("%s: ", GetTranslation("Value")));
    m_pValChoiceMenu->label(CreateText("%s: ", GetTranslation("Value")));
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
    if ((m_pCurrentParamEntry->param_type == PTYPE_STRING) ||
        (m_pCurrentParamEntry->param_type == PTYPE_DIR))
    {
        m_pValChoiceMenu->hide();
        m_pParamInput->show();
        m_pParamInput->value(m_pCurrentParamEntry->value.c_str());
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
                m_pValChoiceMenu->add(p->c_str());
                if (*p == m_pCurrentParamEntry->value) m_pValChoiceMenu->value(s);
            }
        }
        m_pParamInput->hide();
        m_pValChoiceMenu->show();
    }
    m_pDescriptionOutput->value(m_pCurrentParamEntry->description.c_str());
    
    const char *str = m_pCurrentParamEntry->defaultval.c_str();
    if (m_pCurrentParamEntry->param_type == PTYPE_BOOL)
    {
        if (m_pCurrentParamEntry->defaultval == "true") str = GetTranslation("Enabled");
        else str = GetTranslation("Disabled");
    }
    m_pDefaultValBox->label(CreateText(GetTranslation("Default: %s"), str));
    printf("Params: %s\n", GetParameters(pCommandEntry).c_str());
}

void CSetParamsScreen::SetValue(const std::string &str)
{
    if (m_pCurrentParamEntry->param_type == PTYPE_BOOL)
    {
        if (str == "Enable") m_pCurrentParamEntry->value = "true";
        else m_pCurrentParamEntry->value = "false";
    }
    else
        m_pCurrentParamEntry->value = str;
}

void CSetParamsScreen::ParamBrowserCB(Fl_Widget *w, void *p)
{
    std::map<std::string, param_entry_s *>::iterator it;
    short s=0, value=((Fl_Hold_Browser*)w)->value();
    command_entry_s *pCommandEntry = ((command_entry_s *)((Fl_Hold_Browser*)w)->data(value));

    for (it=pCommandEntry->parameter_entries.begin(); s<(value-1); it++, s++);
    
    printf("Param selection: %s\n", it->first.c_str());
    ((CSetParamsScreen *)p)->SetInput(it->first.c_str(), pCommandEntry);
}

void CSetParamsScreen::ValChoiceMenuCB(Fl_Widget *w, void *p)
{
    ((CSetParamsScreen *)p)->SetValue(((Fl_Menu_*)w)->mvalue()->text);
    printf("Value: %s\n", ((Fl_Menu_*)w)->mvalue()->text);
}

void CSetParamsScreen::ParamInputCB(Fl_Widget *w, void *p)
{
    ((CSetParamsScreen *)p)->SetValue(((Fl_Input*)w)->value());
    printf("Value: %s\n", (((Fl_Input*)w)->value()));
}

// -------------------------------------
// Base Install screen
// -------------------------------------

void CInstallFilesScreen::ClearPassword()
{
    if (!m_szPassword) return;
    
    for (int i=0;i<strlen(m_szPassword);i++) m_szPassword[i] = 0;
    free(m_szPassword);
}

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
    
    m_pAskPassWindow = new Fl_Window(400, 190, "Password dialog");
    m_pAskPassWindow->set_modal();
    m_pAskPassWindow->begin();
    
    m_pAskPassBox = new Fl_Box(10, 20, 370, 40, "This installation requires root(administrator) privileges in order to "
                                                "continue.\nPlease enter the password of the root user.");
    m_pAskPassBox->align(FL_ALIGN_WRAP);
    
    m_pAskPassInput = new Fl_Secret_Input(100, 90, 250, 25, "Password: ");
    m_pAskPassInput->take_focus();
    
    m_pAskPassOKButton = new Fl_Button(60, 150, 100, 25, "OK");
    m_pAskPassOKButton->callback(AskPassOKButtonCB, this);
    
    m_pAskPassCancelButton = new Fl_Button(240, 150, 100, 25, "Cancel");
    m_pAskPassCancelButton->callback(AskPassCancelButtonCB, this);
    
    m_pAskPassWindow->end();
    
    m_pGroup->end();

    return m_pGroup;
}

bool CInstallFilesScreen::Activate()
{
    if (m_SUHandler.NeedPassword())
    {
        // Check if we need root access
        for (std::list<command_entry_s *>::iterator it=InstallInfo.command_entries.begin();
             it!=InstallInfo.command_entries.end(); it++)
        {
            if ((*it)->need_root != NO_ROOT)
            {
                 // Command may need root permission, check if it is so
                if ((*it)->need_root == DEPENDED_ROOT)
                {
                    param_entry_s *p = GetParamVar((*it)->dep_param);
                    if (p && access(p->value.c_str(), W_OK) != 0)
                    {
                        (*it)->need_root = NEED_ROOT;
                    }
                    else continue;
                }
            
                if (m_szPassword) continue; // No need to ask pass more than once...
                
                while(true)
                {
                    ClearPassword();
                
                    m_pAskPassWindow->hotspot(m_pAskPassOKButton);
                    m_pAskPassWindow->take_focus();
                    m_pAskPassWindow->show();

                    while(m_pAskPassWindow->visible()) Fl::wait();
            
                    // Check if password is invalid
                    if (!m_szPassword)
                    {
                        if (fl_ask(GetTranslation("Root access is required to continue\nAbort installation?")))
                            EndProg(-1);
                    }
                    else
                    {
                        if (m_SUHandler.TestSU(m_szPassword))
                            break;
                        
                        // Some error appeared
                        if (m_SUHandler.GetError() == CLibSU::SU_ERROR_INCORRECTPASS)
                            fl_alert(GetTranslation("Incorrect password given for root user.\nPlease re-type."));
                        else
                        {
                            fl_alert(GetTranslation("Error: Couldn't use su to gain root access.\n"
                                                    "Make sure you can use su(adding your user to the wheel group may help."));
                            EndProg(-1);
                        }
                    }
                }
            }
        }
    }
    
    m_SUHandler.SetOutputFunc(SUOutputHandler, this);
    m_SUHandler.SetPath("/bin:/usr/bin:/usr/local/bin");
    m_SUHandler.SetUser("root");
    m_SUHandler.SetTerminalOutput(false);
    
    if (chdir(InstallInfo.dest_dir.c_str())); // UNDONE
    InstallFiles = true;
    //Fl::add_idle(CInstallFilesBase::stat_inst, this);
    pPrevButton->deactivate();
    pNextButton->deactivate();
    
    Install();
    return true;
}

void CInstallFilesScreen::UpdateLang()
{
    m_pProgress->label(GetTranslation("Install progress"));
    //m_pDisplay->label(GetTranslation("Status: %s"));
}

void CInstallFilesScreen::AppendText(const char *txt)
{
    m_pDisplay->insert(txt);
    m_pDisplay->show_insert_position();
}

void CInstallFilesScreen::SetPassword(bool unset)
{
    ClearPassword();
    
    const char *passwd = m_pAskPassInput->value();

    if (!unset)
        m_szPassword = strdup(passwd);

    if (passwd && passwd[0])
    {
        // Can't use FLTK's replace() to delete input field text, since it stores undo info
        int length = strlen(passwd);
        
        char str0[length];
        for (int i=0;i<length;i++) str0[i] = 0;
        m_pAskPassInput->value(str0);
        
        // Force clean temp inputfield string
        char *str = const_cast<char *>(passwd);
        for(int i=0;i<strlen(str);i++) str[i] = 0;
    }
    
    m_pAskPassWindow->hide();
    m_pAskPassInput->value(NULL);
}

void CInstallFilesScreen::AskPassOKButtonCB(Fl_Widget *w, void *p)
{
    ((CInstallFilesScreen *)p)->SetPassword(false);
}

void CInstallFilesScreen::AskPassCancelButtonCB(Fl_Widget *w, void *p)
{
    ((CInstallFilesScreen *)p)->SetPassword(true);
}


void CInstallFilesScreen::Install()
{
    bool RootNeedPW = m_SUHandler.NeedPassword();
    char curfile[256], text[300];
    
    ChangeStatusText(GetTranslation("Copying files"));
    while(1)
    {
        m_sPercent = ExtractArchive(curfile);
    
        sprintf(text, "Extracting file: %s\n", curfile);
        AppendText(text);

        if (m_sPercent==-1) EndProg(-2);

        if (m_sPercent==100)
        {
            m_sPercent = 0;
            AppendText("Done!\n");
            break;
        }
    
        UpdateStatusBar();
        Fl::wait(0.0); // Update screen
    }

    for (std::list<command_entry_s*>::iterator it=InstallInfo.command_entries.begin();
            it!=InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->command.empty()) continue;
        
        std::string command = (*it)->command + " " + GetParameters(*it);
    
        AppendText(CreateText("\nExecute: %s\n\n", command.c_str()));
        ChangeStatusText(GetTranslation((*it)->description.c_str()));

        if (RootNeedPW && (*it)->need_root == NEED_ROOT)
        {
            m_SUHandler.SetCommand(command);
            m_SUHandler.ExecuteCommand(m_szPassword);
        }
        else
        {
            FILE *pPipe = popen(command.c_str(), "r");
            if (pPipe)
            {
                char buf[1024];
                while(fgets(buf, sizeof(buf), pPipe))
                {
                    AppendText(buf);
                    Fl::wait(0.0); // Update screen
                }
                pclose(pPipe);
            }
            else
            {
                // UNDONE
            }
        }
        m_sPercent += (1.0f/(float)InstallInfo.command_entries.size())*100.0f;
        UpdateStatusBar();
    }
    
    m_sPercent = 100;
    UpdateStatusBar();
    ChangeStatusText(GetTranslation("Done"));
    InstallFiles = false;
    fl_message(GetTranslation("Installation of %s complete!"), InstallInfo.program_name);
    ClearPassword();
    //EndProg(0);
}
