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

#ifndef FLTK_H
#define FLTK_H

#include <sys/wait.h>

#include "main.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Wizard.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Tabs.H>

#define MAIN_WINDOW_W 600
#define MAIN_WINDOW_H 400

class CBaseScreen;
class CUninstallWindow;

class CAskPassWindow
{
    Fl_Window *m_pAskPassWindow;
    Fl_Box *m_pAskPassBox;
    Fl_Secret_Input *m_pAskPassInput;
    Fl_Return_Button *m_pAskPassOKButton;
    Fl_Button *m_pAskPassCancelButton;
    char *m_szPassword;
    
public:
    CAskPassWindow(const char *msg=NULL);

    void SetMsg(const char *msg) { m_pAskPassBox->label(msg); };
    char *Activate(void);
    void SetPassword(bool unset);
    void UpdateLanguage(void);

    static void AskPassOKButtonCB(Fl_Widget *w, void *p) { ((CAskPassWindow *)p)->SetPassword(false); };
    static void AskPassCancelButtonCB(Fl_Widget *w, void *p) { ((CAskPassWindow *)p)->SetPassword(true); };
};

class CFLTKBase: virtual public CMain
{
    Fl_Window *m_pAboutWindow;
    Fl_Text_Display *m_pAboutDisp;
    Fl_Return_Button *m_pAboutOKButton;
    
protected:
    Fl_Window *m_pMainWindow;
    Fl_Button *m_pAboutButton;
    CAskPassWindow *m_pAskPassWindow;
    
    virtual char *GetPassword(const char *str) { m_pAskPassWindow->SetMsg(str); return m_pAskPassWindow->Activate(); };
    virtual void MsgBox(const char *str, ...);
    virtual bool YesNoBox(const char *str, ...);
    virtual int ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...);
    virtual void Warn(const char *str, ...);
    
public:
    CFLTKBase(void);
    virtual ~CFLTKBase(void);

    virtual void UpdateLanguage(void);

    void ShowAbout(bool show);
    void Run(char **argv) { m_pMainWindow->show(1, argv); Fl::run(); };

    static void AboutOKCB(Fl_Widget *, void *p) { ((CFLTKBase *)p)->ShowAbout(false); };
    static void ShowAboutCB(Fl_Widget *, void *p) { ((CFLTKBase *)p)->ShowAbout(true); };
};

class CWelcomeScreen;
class CLicenseScreen;
class CSelectDirScreen;
class CInstallFilesScreen;
class CFinishScreen;

class CInstaller: public CFLTKBase, public CBaseInstall
{
    Fl_Wizard *m_pWizard;
    CWelcomeScreen *m_pWelcomeScreen;
    CLicenseScreen *m_pLicenseScreen;
    CSelectDirScreen *m_pSelectDirScreen;
    CInstallFilesScreen *m_pInstallFilesScreen;
    CFinishScreen *m_pFinishScreen;
    std::list<CBaseScreen *> m_ScreenList;
    
protected:
    virtual void ChangeStatusText(const char *str);
    virtual void AddInstOutput(const std::string &str);
    virtual void SetProgress(int percent);
    virtual void InstallThink(void) { Fl::wait(0.0f); };
    
public:
    bool m_bInstallFiles;
    Fl_Button *m_pCancelButton;
    Fl_Button *m_pPrevButton;
    Fl_Button *m_pNextButton;

    CInstaller(void) : m_bInstallFiles(false) { };
    virtual ~CInstaller(void);

    virtual bool InitLua(void);
    virtual bool Init(int argc, char **argv);
    virtual void Install(void);
    virtual void UpdateLanguage(void);
    virtual CBaseCFGScreen *CreateCFGScreen(const char *title);
    
    void Prev(void);
    void Next(void);

    static void WizCancelCB(Fl_Widget *, void *p);
    static void WizPrevCB(Fl_Widget *, void *p) { ((CInstaller *)p)->Prev(); };
    static void WizNextCB(Fl_Widget *, void *p) { ((CInstaller *)p)->Next(); };
};

class CBaseLuaWidget
{
    std::string m_szDescription;
    int m_iStartX, m_iStartY, m_iWidth, m_iHeight;
    
    void MakeTitle(void); // How many lines does the title use? (Max 2)
    
protected:
    Fl_Group *m_pGroup;
    Fl_Box *m_pBox;
    
    CBaseLuaWidget(int x, int y, int w, int h, const char *desc);
    virtual ~CBaseLuaWidget(void) { };

    virtual Fl_Group *Create(void);
    
    int DescHeight(void) { return (m_pBox) ? m_pBox->h() : 0; };
    static int TitleHeight(int w, const char *desc);

public:
    virtual void UpdateLanguage(void);
};

class CLuaInputField: public CBaseLuaInputField, public CBaseLuaWidget
{
    std::string m_szLabel, m_szValue;
    Fl_Box *m_pLabel;
    Fl_Input *m_pInput;
    int m_iMax;
    static int m_iFieldHeight;
    
public:
    CLuaInputField(int x, int y, int w, int h, const char *label, const char *desc, const char *val, int max);

    virtual Fl_Group *Create(void);
    virtual void UpdateLanguage(void);
    virtual const char *GetValue(void) { return m_pInput->value(); };
    virtual void SetSpacing(int percent);

    static int CalcHeight(int w, const char *desc);
};

class CLuaCheckbox: public CBaseLuaCheckbox, public CBaseLuaWidget
{
    const std::vector<std::string> m_Options;
    std::vector<Fl_Check_Button *> m_Buttons;
    static int m_iButtonHeight, m_iButtonSpace, m_iBoxSpace;
    
public:
    CLuaCheckbox(int x, int y, int w, int h, const char *desc, const std::vector<std::string> &l);

    virtual Fl_Group *Create(void);
    virtual void UpdateLanguage(void);
    virtual bool Enabled(int n) { return m_Buttons.at(n-1)->value(); };
    virtual void Enable(int n) { m_Buttons.at(n-1)->value(1); };

    static int CalcHeight(int w, const char *desc, const std::vector<std::string> &l);
};

class CLuaRadioButton: public CBaseLuaRadioButton, public CBaseLuaWidget
{
    const std::vector<std::string> m_Options;
    std::vector<Fl_Round_Button *> m_Buttons;
    static int m_iButtonHeight, m_iButtonSpace, m_iBoxSpace;
    
public:
    CLuaRadioButton(int x, int y, int w, int h, const char *desc, const std::vector<std::string> &l);

    virtual Fl_Group *Create(void);
    virtual void UpdateLanguage(void);
    virtual int EnabledButton(void);
    virtual void Enable(int n) { m_Buttons.at(n-1)->setonly(); };

    static int CalcHeight(int w, const char *desc, const std::vector<std::string> &l);
};

class CLuaDirSelector: public CBaseLuaDirSelector, public CBaseLuaWidget
{
    std::string m_szValue;
    Fl_Input *m_pDirInput;
    Fl_Button *m_pDirButton;
    Fl_File_Chooser *m_pDirChooser;
    static int m_iFieldHeight, m_iButtonWidth;
    
    void OpenDirChooser(void);
    
public:
    CLuaDirSelector(int x, int y, int w, int h, const char *desc, const char *val);

    virtual Fl_Group *Create(void);
    virtual void UpdateLanguage(void);
    virtual const char *GetDir(void) { return m_pDirInput->value(); };
    virtual void SetDir(const char *dir) { m_pDirInput->value(dir); };

    static int CalcHeight(int w, const char *desc);
    static void OpenDirSelWinCB(Fl_Widget *w, void *p) { ((CLuaDirSelector *)p)->OpenDirChooser(); };
};

class CLuaCFGMenu: public CBaseLuaCFGMenu, public CBaseLuaWidget
{
    Fl_Hold_Browser *m_pMenu;
    Fl_Text_Display *m_pDescOutput;
    Fl_Text_Buffer *m_pDescBuffer;
    Fl_Button *m_pBrowseButton;
    Fl_Choice *m_pChoiceMenu;
    Fl_Input *m_pInputField;
    Fl_File_Chooser *m_pDirChooser;
    
    static int m_iMenuHeight, m_iMenuWidth, m_iDescHeight, m_iButtonWidth, m_iButtonHeight;
    
    void CreateDirSelector(void);
    void SetInfo(void);
    void SetInputMethod(void);
    void OpenDir(void);
    void SetString(const char *s);
    void SetChoice(int n);
    
public:
    CLuaCFGMenu(int x, int y, int w, int h, const char *desc);
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLanguage(void);
    virtual void AddVar(const char *name, const char *desc, const char *val, EVarType type, TOptionsType *l=NULL);
    
    static int CalcHeight(int w, const char *desc);
    static void MenuCB(Fl_Widget *w, void *p) { ((CLuaCFGMenu *)p)->SetInfo(); ((CLuaCFGMenu *)p)->SetInputMethod(); };
    static void BrowseCB(Fl_Widget *w, void *p) { ((CLuaCFGMenu *)p)->OpenDir(); };
    static void InputFieldCB(Fl_Widget *w, void *p) { ((CLuaCFGMenu *)p)->SetString(((Fl_Input *)w)->value()); };
    static void ChoiceCB(Fl_Widget *w, void *p) { ((CLuaCFGMenu *)p)->SetChoice(((Fl_Choice *)w)->value()); };
};

class CAppManager: public CFLTKBase, public CBaseAppManager
{
    Fl_Button *m_pUninstallButton, *m_pExitButton;
    Fl_Help_View *m_pInfoOutput;
    Fl_Hold_Browser *m_pAppList;
    Fl_Text_Display *m_pFilesTextDisplay;
    Fl_Text_Buffer *m_pFilesTextBuffer;
    std::vector<app_entry_s *> m_AppVec;
    app_entry_s *m_pCurrentAppEntry;
    CUninstallWindow *m_pUninstallWindow;
    
    friend class CUninstallWindow;
    
protected:
    virtual void AddUninstOutput(const std::string &str);
    virtual void SetProgress(int percent);
    
public:
    CAppManager(void);
    virtual ~CAppManager(void)
    { for (std::vector<app_entry_s*>::iterator it=m_AppVec.begin(); it!=m_AppVec.end(); it++) delete *it; };
    
    void UpdateInfo(bool init);
    void StartUninstall(void);
    
    static void AppListCB(Fl_Widget *, void *p) { ((CAppManager *)p)->UpdateInfo(false); };
    static void UninstallCB(Fl_Widget *, void *p) { ((CAppManager *)p)->StartUninstall(); };
    static void ExitCB(Fl_Widget *, void *) { EndProg(); };
};


// -------------------------
// Installer screens
// -------------------------

class CBaseScreen
{
protected:
    Fl_Group *m_pGroup;
    CInstaller *m_pOwner;
    
    int CenterX(int w) { return ((MAIN_WINDOW_W-w)/2); };
    int CenterX(int w, const char *label, bool left);
    
    // For centering 2 widgets
    int CenterX2(int w) { return ((MAIN_WINDOW_W-w)/3); };
    int CenterX2(int w, const char *l1, const char *l2, bool left1, bool left2);
    
    int CenterY(int h) { return ((m_pGroup->y()+m_pGroup->h())-h)/2; };
    
public:
    CBaseScreen(CInstaller *owner) : m_pGroup(NULL), m_pOwner(owner) { };
    virtual ~CBaseScreen(void) { };
    
    virtual Fl_Group *Create(void) { return (m_pGroup = new Fl_Group(20, 20, (MAIN_WINDOW_W-40), (MAIN_WINDOW_H-60), NULL)); };
    Fl_Group *GetGroup(void) const { return m_pGroup; };
    virtual void UpdateLang(void) { }; // Called after language is changed
    virtual bool Prev(void) { return true; };
    virtual bool Next(void) { return true; };
    virtual bool Activate(void) { return true; };
};

class CLangScreen: public CBaseScreen
{
    Fl_Choice *m_pChoiceMenu;
    
public:
    CLangScreen(CInstaller *owner) : CBaseScreen(owner) { };
    virtual Fl_Group *Create(void);
    virtual bool Next(void);
    virtual bool Activate(void);
    void SetLang(void) { m_pOwner->m_szCurLang = m_pChoiceMenu->mvalue()->text; };
    
    static void LangMenuCB(Fl_Widget *w, void *p) { ((CLangScreen *)p)->SetLang(); };
};

class CWelcomeScreen: public CBaseScreen
{
    Fl_Shared_Image *m_pImage;
    Fl_Box *m_pImageBox;
    Fl_Text_Display *m_pDisplay;
    Fl_Text_Buffer *m_pBuffer;
    bool m_bHasText;
    
public:
    CWelcomeScreen(CInstaller *owner) : CBaseScreen(owner), m_bHasText(false) { };
    
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
    CLicenseScreen(CInstaller *owner) : CBaseScreen(owner), m_bHasText(false) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Prev(void) { m_pOwner->m_pNextButton->activate(); return true; };
    virtual bool Activate(void);
    
    void LicenseCheck(bool on) { (on)?m_pOwner->m_pNextButton->activate():m_pOwner->m_pNextButton->deactivate(); };
        
    static void LicenseCheckCB(Fl_Widget *w, void *p) { ((CLicenseScreen *)p)->LicenseCheck(((Fl_Button*)w)->value()); };
};

class CSelectDirScreen: public CBaseScreen
{
    Fl_File_Chooser *m_pDirChooser;
    Fl_Box *m_pBox;
    Fl_Button *m_pSelDirButton;
    Fl_Output *m_pSelDirInput;
    
public:
    CSelectDirScreen(CInstaller *owner) : CBaseScreen(owner), m_pDirChooser(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Next(void);
    virtual bool Activate(void) { return (m_pOwner->m_InstallInfo.dest_dir_type == "select"); };
    
    void OpenDirChooser(void);
    static void OpenDirSelWinCB(Fl_Widget *w, void *p) { ((CSelectDirScreen *)p)->OpenDirChooser(); };
};

class CInstallFilesScreen: public CBaseScreen
{
    Fl_Progress *m_pProgress;
    Fl_Text_Buffer *m_pBuffer;
    Fl_Text_Display *m_pDisplay;
    
public:
    CInstallFilesScreen(CInstaller *owner) : CBaseScreen(owner) { };
    virtual ~CInstallFilesScreen(void) { };
    
    virtual Fl_Group *Create(void);
    virtual bool Activate(void) { m_pOwner->Install(); return true; };
    virtual void UpdateLang(void);
    
    void AppendText(const char *txt);
    void AppendText(const std::string &txt) { AppendText(txt.c_str()); };
    void ChangeStatusText(const char *txt);
    void SetProgress(int percent) { m_pProgress->value(percent); Fl::wait(0.0); };
};

class CFinishScreen: public CBaseScreen
{
    Fl_Text_Display *m_pDisplay;
    Fl_Text_Buffer *m_pBuffer;
    bool m_bHasText;
    
public:
    CFinishScreen(CInstaller *owner) : CBaseScreen(owner), m_bHasText(false) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Activate(void);
};

class CCFGScreen: public CBaseScreen, public CBaseCFGScreen
{
    std::string m_szTitle;
    Fl_Box *m_pBoxTitle;
    int m_iStartX, m_iStartY;
    CCFGScreen *m_pNextScreen; 
    std::vector<CBaseLuaWidget *> m_LuaWidgets;
    int m_iLinkedScrNr, m_iLinkedScrMax;
    
    friend class CInstaller;
    
    void SetTitle(void);
    
public:
    CCFGScreen(CInstaller *owner, const char *title) : CBaseScreen(owner), m_szTitle(title), m_pNextScreen(NULL),
                                                       m_iLinkedScrNr(0), m_iLinkedScrMax(0) { };

    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Activate(void);
    
    virtual CBaseLuaInputField *CreateInputField(const char *label, const char *desc, const char *val, int max);
    virtual CBaseLuaCheckbox *CreateCheckbox(const char *desc, const std::vector<std::string> &l);
    virtual CBaseLuaRadioButton *CreateRadioButton(const char *desc, const std::vector<std::string> &l);
    virtual CBaseLuaDirSelector *CreateDirSelector(const char *desc, const char *val);
    virtual CBaseLuaCFGMenu *CreateCFGMenu(const char *desc);
    
    void SetCounter(int n, int max) { m_iLinkedScrNr = n; m_iLinkedScrMax = max; };
};

// -------------------------
// AppManager classes
// -------------------------

class CUninstallWindow
{
    CAppManager *m_pOwner;
    Fl_Window *m_pWindow;
    Fl_Progress *m_pProgress;
    Fl_Text_Buffer *m_pBuffer;
    Fl_Text_Display *m_pDisplay;
    Fl_Button *m_pOKButton;
    
public:
    CUninstallWindow(CAppManager *owner);
    
    bool Start(app_entry_s *pApp);
    void Close(void) { m_pWindow->hide(); };
    void AddOutput(const std::string &str);
    void SetProgress(int percent) { m_pProgress->value(percent); Fl::wait(0.0); };
     
    static void OKButtonCB(Fl_Widget *w, void *p) { ((CUninstallWindow *)p)->Close(); };
};

#ifndef RELEASE
void debugline(const char *t, ...);
#endif

#endif
