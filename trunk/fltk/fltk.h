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
#include <FL/Fl_Input.H>
#include <FL/Fl_Progress.H>

#define MAIN_WINDOW_W 600
#define MAIN_WINDOW_H 400

void LangMenuCB(Fl_Widget *w, void *);
void LicenseCheckCB(Fl_Widget *w, void *);
void OpenDirSelWinCB(Fl_Widget *w, void *p);
void UpdateLanguage(void);
void EndProg(int exitcode);

extern Fl_Button *pPrevButton;
extern Fl_Button *pNextButton;
extern bool InstallFiles;

class CBaseScreen
{
protected:
    Fl_Group *pGroup;
    
public:
    CBaseScreen(void) : pGroup(NULL) { };
    
    virtual Fl_Group *Create(void) = NULL;
    Fl_Group *GetGroup(void) const { return pGroup; };
    virtual void UpdateLang(void) { }; // Called after language is changed
    virtual bool Prev(void) { return true; };
    virtual bool Next(void) { return true; };
    virtual void Activate(void) { };
};

class CLangScreen: public CBaseScreen
{
    Fl_Menu_Item *pMenuItems;
    Fl_Choice *pChoiceMenu;
    
public:
    CLangScreen(void) : CBaseScreen(), pMenuItems(NULL), pChoiceMenu(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual bool Next(void);
    virtual void Activate(void) { pPrevButton->deactivate(); };
};

class CWelcomeScreen: public CBaseScreen
{
    Fl_Text_Display *pDisplay;
    
public:
    CWelcomeScreen(void) : CBaseScreen(), pDisplay(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
};

class CLicenseScreen: public CBaseScreen
{
    Fl_Text_Display *pDisplay;
    Fl_Check_Button *pCheckButton;
    
public:
    CLicenseScreen(void) : CBaseScreen(), pDisplay(NULL), pCheckButton(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Prev(void) { pNextButton->activate(); return true; };
    virtual void Activate(void);
};

class CSelectDirScreen: public CBaseScreen
{
    Fl_File_Chooser *pDirChooser;
    Fl_Box *pBox;
    Fl_Button *pSelDirButton;
    Fl_Input *pSelDirInput;
    
public:
    CSelectDirScreen(void) : CBaseScreen(), pDirChooser(NULL), pBox(NULL), pSelDirButton(NULL), pSelDirInput(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Next(void);
    void OpenDirChooser(void);
};

class CInstallFilesScreen: public CBaseScreen
{
    Fl_Progress *pProgress;
    Fl_Text_Display *pDisplay;
    Fl_Text_Buffer *pBuffer;
    short Percent;
    
public:
    CInstallFilesScreen(void) : CBaseScreen(), pProgress(NULL), pDisplay(NULL), pBuffer(NULL), Percent(0) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual void Activate(void);
    void Install(void);
    static void stat_inst(void *p) { if (InstallFiles) ((CInstallFilesScreen *)p)->Install(); };
};

#endif
