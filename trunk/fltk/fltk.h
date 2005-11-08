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
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Hold_Browser.H>

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
    virtual bool Activate(void) { return true; };
    virtual bool Enabled(void) { return true; };
};

class CLangScreen: public CBaseScreen
{
    Fl_Menu_Item *pMenuItems;
    Fl_Choice *pChoiceMenu;
    
public:
    CLangScreen(void) : CBaseScreen(), pMenuItems(NULL), pChoiceMenu(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual bool Next(void);
    virtual bool Activate(void) { pPrevButton->deactivate(); return true; };
};

class CWelcomeScreen: public CBaseScreen
{
    Fl_Text_Display *pDisplay;
    Fl_Text_Buffer *pBuffer;
    bool HasText;
    
public:
    CWelcomeScreen(void) : CBaseScreen(), pDisplay(NULL), pBuffer(NULL), HasText(false) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Activate(void) { return HasText; };
};

class CLicenseScreen: public CBaseScreen
{
    Fl_Text_Display *pDisplay;
    Fl_Check_Button *pCheckButton;
    Fl_Text_Buffer *pBuffer;
    bool HasText;
    
public:
    CLicenseScreen(void) : CBaseScreen(), pDisplay(NULL), pCheckButton(NULL), pBuffer(NULL), HasText(false) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Prev(void) { pNextButton->activate(); return true; };
    virtual bool Activate(void);
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

class CSetParamsScreen: public CBaseScreen
{
    Fl_Box *m_pBoxTitle, *m_pDefaultValBox;
    Fl_Input *m_pParamInput;
    Fl_Menu_Item *m_pValMenuItems;
    Fl_Choice *m_pValChoiceMenu;
    Fl_Hold_Browser *m_pChoiceBrowser;
    Fl_Multiline_Output *m_pDescriptionOutput;
    
    compile_entry_s::param_entry_s *m_pCurrentParamEntry;
    
public:
    CSetParamsScreen(void) : CBaseScreen(), m_pBoxTitle(NULL), m_pDefaultValBox(NULL), m_pParamInput(NULL),
                             m_pValChoiceMenu(NULL), m_pChoiceBrowser(NULL), m_pDescriptionOutput(NULL),
                             m_pCurrentParamEntry(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Activate(void);
    //virtual bool Next(void);
    
    void SetInput(const char *txt, compile_entry_s *pCompileEntry);
    void SetValue(const std::string &str);
    
    static void ParamBrowserCB(Fl_Widget *w, void *p);
    static void ValChoiceMenuCB(Fl_Widget *w, void *p);
    static void ParamInputCB(Fl_Widget *w, void *p);
};

class CInstallFilesBase: public CBaseScreen
{
protected:
    Fl_Progress *pProgress;
    Fl_Text_Buffer *pBuffer;
    Fl_Text_Display *pDisplay;
    short Percent;
    CLibSU SUHandler;
    
public:
    CInstallFilesBase(void) : CBaseScreen(), pProgress(NULL), pDisplay(NULL), pBuffer(NULL), Percent(0) { };
    
    virtual Fl_Group *Create(void);
    virtual bool Activate(void);
    virtual void UpdateLang(void);
    virtual void Install(void) { };
    
    void UpdateStatusBar(void) { pProgress->value(Percent); };
    void AppendText(const char *txt);
    void ChangeStatusText(const char *txt) { pDisplay->label(CreateText(GetTranslation("Status: %s"), txt)); };
    
    static void stat_inst(void *p) { if (InstallFiles) ((CInstallFilesBase *)p)->Install(); };
    static void SUOutputHandler(const char *msg, void *p) { ((CInstallFilesBase *)p)->AppendText(msg); Fl::wait(); };
};

class CSimpleInstallScreen: public CInstallFilesBase
{
public:
    virtual bool Activate(void);
    virtual void Install(void);
};

class CCompileInstallScreen: public CInstallFilesBase
{
    bool m_bCompiling;
    std::list<compile_entry_s *>::iterator m_CurrentIterator;
    std::list<std::string>::iterator m_CurrentCommandIterator;
    
public:
    CCompileInstallScreen(void) : CInstallFilesBase(), m_bCompiling(false) { };
    virtual bool Activate(void);
    virtual void Install(void);
};

#endif
