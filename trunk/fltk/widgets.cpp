#include "fltk.h"

// -------------------------------------
// Language selection widget
// -------------------------------------

Fl_Group *CLangWidget::Create(void)
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

void CLangWidget::Next()
{
    pPrevButton->activate();
    ReadLang();
    UpdateLanguage();
}

// -------------------------------------
// Welcome text display widget
// -------------------------------------

Fl_Group *CWelcomeWidget::Create(void)
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

void CWelcomeWidget::UpdateLang()
{
    pDisplay->label(GetTranslation("Welcome"));
}

// -------------------------------------
// License agreement widget
// -------------------------------------

Fl_Group *CLicenseWidget::Create(void)
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

void CLicenseWidget::UpdateLang()
{
    pDisplay->label(GetTranslation("License Agreement"));
}

void CLicenseWidget::Activate()
{
    if (!pCheckButton->value()) pNextButton->deactivate();
}

// -------------------------------------
// Destination dir selector widget
// -------------------------------------

Fl_Group *CSelectDirWidget::Create()
{
    pDirChooser = new Fl_File_Chooser(InstallInfo.dest_dir, "*", (Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE),
                                      "Select destination directory");
    pDirChooser->hide();
    pDirChooser->preview(false);
    pDirChooser->previewButton->hide();
    
    pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    pGroup->begin();

    pBox = new Fl_Box((MAIN_WINDOW_W-260)/2, 40, 260, 100, "Select destination directory");
    pSelDirInput = new Fl_Input(80, ((MAIN_WINDOW_H-60)-20)/2, 300, 25, "path: ");
    pSelDirInput->value(InstallInfo.dest_dir);
    pSelDirButton = new Fl_Button((MAIN_WINDOW_W-200), ((MAIN_WINDOW_H-60)-20)/2, 160, 25, "Select directory");
    pSelDirButton->callback(OpenDirSelWinCB, this);
    
    pGroup->end();
    return pGroup;
}

void CSelectDirWidget::OpenDirChooser(void)
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
