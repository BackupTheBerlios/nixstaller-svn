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
#include <FL/Fl_Output.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Hold_Browser.H>

#define MAIN_WINDOW_W 600
#define MAIN_WINDOW_H 400

void UpdateLanguage(void);
void EndProg(int exitcode);

extern Fl_Button *pPrevButton;
extern Fl_Button *pNextButton;
extern bool InstallFiles;

class CBaseScreen
{
protected:
    Fl_Group *m_pGroup;
    
public:
    CBaseScreen(void) : m_pGroup(NULL) { };
    
    virtual Fl_Group *Create(void) = NULL;
    Fl_Group *GetGroup(void) const { return m_pGroup; };
    virtual void UpdateLang(void) { }; // Called after language is changed
    virtual bool Prev(void) { return true; };
    virtual bool Next(void) { return true; };
    virtual bool Activate(void) { return true; };
};

class CLangScreen: public CBaseScreen
{
    Fl_Menu_Item *m_pMenuItems;
    Fl_Choice *m_pChoiceMenu;
    
public:
    virtual Fl_Group *Create(void);
    virtual bool Next(void);
    virtual bool Activate(void) { pPrevButton->deactivate(); return true; };
    
    static void LangMenuCB(Fl_Widget *w, void *) { InstallInfo.cur_lang = ((Fl_Menu_*)w)->mvalue()->text; };
};

class CWelcomeScreen: public CBaseScreen
{
    Fl_Text_Display *m_pDisplay;
    Fl_Text_Buffer *m_pBuffer;
    bool m_bHasText;
    
public:
    CWelcomeScreen(void) : CBaseScreen(), m_bHasText(false) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Activate(void) { return m_bHasText; };
};

class CLicenseScreen: public CBaseScreen
{
    Fl_Text_Display *m_pDisplay;
    Fl_Check_Button *m_pCheckButton;
    Fl_Text_Buffer *m_pBuffer;
    bool m_bHasText;
    
public:
    CLicenseScreen(void) : CBaseScreen(), m_bHasText(false) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Prev(void) { pNextButton->activate(); return true; };
    virtual bool Activate(void);
    
    static void LicenseCheckCB(Fl_Widget *w, void *)
                { (((Fl_Button*)w)->value())?pNextButton->activate():pNextButton->deactivate(); };
};

class CSelectDirScreen: public CBaseScreen
{
    Fl_File_Chooser *m_pDirChooser;
    Fl_Box *m_pBox;
    Fl_Button *m_pSelDirButton;
    Fl_Output *m_pSelDirInput;
    
public:
    CSelectDirScreen(void) : CBaseScreen(), m_pDirChooser(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Next(void);
    
    void OpenDirChooser(void);
    static void OpenDirSelWinCB(Fl_Widget *w, void *p) { ((CSelectDirScreen *)p)->OpenDirChooser(); };
};

class CSetParamsScreen: public CBaseScreen
{
    Fl_Box *m_pBoxTitle, *m_pDefaultValBox;
    Fl_Input *m_pParamInput;
    Fl_Menu_Item *m_pValMenuItems;
    Fl_Choice *m_pValChoiceMenu;
    Fl_Hold_Browser *m_pChoiceBrowser;
    Fl_Multiline_Output *m_pDescriptionOutput;
    
    command_entry_s::param_entry_s *m_pCurrentParamEntry;
    
public:
    CSetParamsScreen(void) : CBaseScreen(), m_pCurrentParamEntry(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Activate(void);
    //virtual bool Next(void);
    
    void SetInput(const char *txt, command_entry_s *pCommandEntry);
    void SetValue(const std::string &str);
    
    static void ParamBrowserCB(Fl_Widget *w, void *p);
    static void ValChoiceMenuCB(Fl_Widget *w, void *p);
    static void ParamInputCB(Fl_Widget *w, void *p);
};

class CInstallFilesBase: public CBaseScreen
{
protected:
    Fl_Progress *m_pProgress;
    Fl_Text_Buffer *m_pBuffer;
    Fl_Text_Display *m_pDisplay;
    
    Fl_Window *m_pAskPassWindow;
    Fl_Box *m_pAskPassBox;
    Fl_Secret_Input *m_pAskPassInput;
    Fl_Button *m_pAskPassOKButton, *m_pAskPassCancelButton;
    
    short m_sPercent;
    CLibSU m_SUHandler;
    char *m_szPassword;
    
    void ClearPassword(void);
    
public:
    CInstallFilesBase(void) : CBaseScreen(), m_sPercent(0), m_szPassword(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual bool Activate(void);
    virtual void UpdateLang(void);
    virtual void Install(void) { };
    
    void UpdateStatusBar(void) { m_pProgress->value(m_sPercent); };
    void AppendText(const char *txt);
    void ChangeStatusText(const char *txt) { m_pDisplay->label(CreateText(GetTranslation("Status: %s"), txt)); };
    void SetPassword(bool unset);
    
    static void stat_inst(void *p) { if (InstallFiles) ((CInstallFilesBase *)p)->Install(); };
    static void SUOutputHandler(const char *msg, void *p) { ((CInstallFilesBase *)p)->AppendText(msg); Fl::wait(); };
    static void AskPassOKButtonCB(Fl_Widget *w, void *p);
    static void AskPassCancelButtonCB(Fl_Widget *w, void *p);
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
    std::list<command_entry_s *>::iterator m_CurrentIterator;
    std::list<std::string>::iterator m_CurrentCommandIterator;
    
public:
    CCompileInstallScreen(void) : CInstallFilesBase(), m_bCompiling(false) { };
    virtual bool Activate(void);
    virtual void Install(void);
};

#endif
