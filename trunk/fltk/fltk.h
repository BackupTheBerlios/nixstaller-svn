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

#define MAIN_WINDOW_W 600
#define MAIN_WINDOW_H 400

class CBaseWidget
{
protected:
    Fl_Group *pGroup;
    
public:
    CBaseWidget(void) : pGroup(NULL) { };
    
    virtual Fl_Group *Create(void) = NULL;
    Fl_Group *GetGroup(void) const { return pGroup; };
    virtual void UpdateLang(void) { }; // Called after language is changed
    virtual void Activate(void) { };
};

class CLangWidget: public CBaseWidget
{
    Fl_Menu_Item *pMenuItems;
    Fl_Choice *pChoiceMenu;
    
public:
    CLangWidget(void) : CBaseWidget(), pMenuItems(NULL), pChoiceMenu(NULL) { };
    
    virtual Fl_Group *Create(void);
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
    
public:
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual void Activate(void);
};

#endif
