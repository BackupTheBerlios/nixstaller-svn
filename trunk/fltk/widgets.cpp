#include "fltk.h"

extern void LangMenuCB(Fl_Widget *w, void *);

extern Fl_Button *pNextButton;

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
    
    pDisplay = new Fl_Text_Display(60, 60, (MAIN_WINDOW_W-90), (MAIN_WINDOW_H-120), "License Agreement");
    pDisplay->buffer(buffer);
    
    pGroup->end();
    return pGroup;
}

void CLicenseWidget::UpdateLang()
{
    pDisplay->label(GetTranslation("License Agreement"));
}

void CLicenseWidget::Activate()
{
    pNextButton->deactivate();
}
