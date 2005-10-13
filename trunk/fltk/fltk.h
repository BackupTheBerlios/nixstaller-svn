#ifndef GLTK_H
#define GLTK_H

#include "main.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Wizard.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <Fl/Fl_Input.H>

#define MAIN_WINDOW_W 600
#define MAIN_WINDOW_H 400

void LangMenuCB(Fl_Widget *w, void *);
void LicenseCheckCB(Fl_Widget *w, void *);
void OpenDirSelWinCB(Fl_Widget *w, void *p);
void UpdateLanguage(void);

extern Fl_Button *pPrevButton;
extern Fl_Button *pNextButton;

class CBaseWidget
{
protected:
    Fl_Group *pGroup;
    
public:
    CBaseWidget(void) : pGroup(NULL) { };
    
    virtual Fl_Group *Create(void) = NULL;
    Fl_Group *GetGroup(void) const { return pGroup; };
    virtual void UpdateLang(void) { }; // Called after language is changed
    virtual void Prev(void) { };
    virtual void Next(void) { };
    virtual void Activate(void) { };
};

class CLangWidget: public CBaseWidget
{
    Fl_Menu_Item *pMenuItems;
    Fl_Choice *pChoiceMenu;
    
public:
    CLangWidget(void) : CBaseWidget(), pMenuItems(NULL), pChoiceMenu(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void Next(void);
    virtual void Activate(void) { pPrevButton->deactivate(); };
};

class CWelcomeWidget: public CBaseWidget
{
    Fl_Text_Display *pDisplay;
    
public:
    CWelcomeWidget(void) : CBaseWidget(), pDisplay(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
};

class CLicenseWidget: public CBaseWidget
{
    Fl_Text_Display *pDisplay;
    Fl_Check_Button *pCheckButton;
    
public:
    CLicenseWidget(void) : CBaseWidget(), pDisplay(NULL), pCheckButton(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual void Prev(void) { pNextButton->activate(); };
    virtual void Activate(void);
};

class CSelectDirWidget: public CBaseWidget
{
    Fl_File_Chooser *pDirChooser;
    Fl_Box *pBox;
    Fl_Button *pSelDirButton;
    Fl_Input *pSelDirInput;
    
public:
    CSelectDirWidget(void) : CBaseWidget(), pDirChooser(NULL), pBox(NULL), pSelDirButton(NULL), pSelDirInput(NULL) { };
    
    virtual Fl_Group *Create(void);
    void OpenDirChooser(void);
};

#endif
