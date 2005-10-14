#include "fltk.h"

// -------------------------------------
// Language selection screen
// -------------------------------------

Fl_Group *CLangScreen::Create(void)
{
    pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    pGroup->begin();
    
    pMenuItems = new Fl_Menu_Item[InstallInfo.languages.size()+1];
    
    new Fl_Box((MAIN_WINDOW_W-260)/2, 40, 260, 100, "Please select a language");
    
    short s=0;
    for (std::list<char*>::iterator p=InstallInfo.languages.begin();p!=InstallInfo.languages.end();p++, s++)
        pMenuItems[s].text = *p;
        
    pMenuItems[s].text = NULL;
    pChoiceMenu = new Fl_Choice(((MAIN_WINDOW_W-60)/2), (MAIN_WINDOW_H-50)/2, 120, 25,"&Language: ");
    pChoiceMenu->menu(pMenuItems);
    pChoiceMenu->callback(LangMenuCB);
    
    pGroup->end();
    
    return pGroup;
}

bool CLangScreen::Next()
{
    pPrevButton->activate();
    ReadLang();
    UpdateLanguage();
    return true;
}

// -------------------------------------
// Welcome text display screen
// -------------------------------------

Fl_Group *CWelcomeScreen::Create(void)
{
    pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    pGroup->begin();
    
    Fl_Text_Buffer *buffer = new Fl_Text_Buffer;
    buffer->loadfile("config/welcome");
    pDisplay = new Fl_Text_Display(60, 60, (MAIN_WINDOW_W-90), (MAIN_WINDOW_H-120), "Welcome");
    pDisplay->buffer(buffer);
    
    pGroup->end();
    
    return pGroup;
}

void CWelcomeScreen::UpdateLang()
{
    pDisplay->label(GetTranslation("Welcome"));
}

// -------------------------------------
// License agreement screen
// -------------------------------------

Fl_Group *CLicenseScreen::Create(void)
{
    Fl_Text_Buffer *buffer = new Fl_Text_Buffer;
    if (buffer->loadfile("config/license")) return NULL; // No license found

    pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    pGroup->begin();
    
    pDisplay = new Fl_Text_Display(60, 60, (MAIN_WINDOW_W-90), (MAIN_WINDOW_H-160), "License Agreement");
    pDisplay->buffer(buffer);
    
    pCheckButton = new Fl_Check_Button((MAIN_WINDOW_W-240)/2, (MAIN_WINDOW_H-80), 240, 25, "I Agree to this license agreement");
    pCheckButton->callback(LicenseCheckCB);
    
    pGroup->end();
    return pGroup;
}

void CLicenseScreen::UpdateLang()
{
    pDisplay->label(GetTranslation("License Agreement"));
    pCheckButton->label(GetTranslation("I Agree to this license agreement"));
}

void CLicenseScreen::Activate()
{
    if (!pCheckButton->value()) pNextButton->deactivate();
}

// -------------------------------------
// Destination dir selector screen
// -------------------------------------

Fl_Group *CSelectDirScreen::Create()
{
    pDirChooser = new Fl_File_Chooser(InstallInfo.dest_dir, "*", (Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE),
                                      "Select destination directory");
    pDirChooser->hide();
    pDirChooser->preview(false);
    pDirChooser->previewButton->hide();
    
    pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    pGroup->begin();

    pBox = new Fl_Box((MAIN_WINDOW_W-260)/2, 40, 260, 100, "Select destination directory");
    pSelDirInput = new Fl_Input(80, ((MAIN_WINDOW_H-60)-20)/2, 300, 25, "dir: ");
    pSelDirInput->value(InstallInfo.dest_dir);
    pSelDirButton = new Fl_Button((MAIN_WINDOW_W-200), ((MAIN_WINDOW_H-60)-20)/2, 160, 25, "Select directory");
    pSelDirButton->callback(OpenDirSelWinCB, this);
    
    pGroup->end();
    return pGroup;
}

void CSelectDirScreen::UpdateLang()
{
    pDirChooser->label(GetTranslation("Select destination directory"));
    pBox->label(GetTranslation("Select destination directory"));
    pSelDirButton->label(GetTranslation("Select directory"));
}

bool CSelectDirScreen::Next()
{
    char temp[128];
    sprintf(temp, GetTranslation("This will install %s to the following directory:"), InstallInfo.program_name);
    return (fl_ask("%s\n%s\n%s", temp, InstallInfo.dest_dir, GetTranslation("Continue?")));
}

void CSelectDirScreen::OpenDirChooser(void)
{
    pDirChooser->show();
    while(pDirChooser->visible()) Fl::wait();
    
    if (pDirChooser->value())
    {
        strncpy(InstallInfo.dest_dir, pDirChooser->value(), 2047);
        InstallInfo.dest_dir[2047] = 0;
        pSelDirInput->value(InstallInfo.dest_dir);
    }
}

// -------------------------------------
// Install progress screen
// -------------------------------------

Fl_Group *CInstallFilesScreen::Create()
{
    pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    pGroup->begin();

    pProgress = new Fl_Progress(50, 60, (MAIN_WINDOW_W-100), 30, "Install progress");
    pProgress->minimum(0);
    pProgress->maximum(100);
    pProgress->value(0);
    
    pBuffer = new Fl_Text_Buffer;
    
    pDisplay = new Fl_Text_Display(50, 110, (MAIN_WINDOW_W-100), (MAIN_WINDOW_H-170), "Status");
    pDisplay->buffer(pBuffer);
    
    pGroup->end();

    return pGroup;
}

void CInstallFilesScreen::UpdateLang()
{
    pProgress->label(GetTranslation("Install progress"));
    pDisplay->label(GetTranslation("Status"));
}

void CInstallFilesScreen::Install()
{
    char curfile[256], text[300];
    Percent = ExtractArchive(curfile);
        
    sprintf(text, "Extracting file: %s\n", curfile);
    pBuffer->append(text);
    pDisplay->move_down();
    if (Percent==-1) EndProg(-2);
    pProgress->value(Percent);
    pDisplay->show_insert_position();
    if (Percent==100)
    {
        pBuffer->append("Done!\n");
        pDisplay->move_down();
        InstallFiles = false;
        fl_message(GetTranslation("Installation of %s complete!"), InstallInfo.program_name);
        EndProg(0);
    }
}

void CInstallFilesScreen::Activate()
{
    chdir(InstallInfo.dest_dir);
    InstallFiles = true;
    pPrevButton->deactivate();
    pNextButton->deactivate();
}
