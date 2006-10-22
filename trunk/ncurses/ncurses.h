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

#ifndef NCURSES_H
#define NCURSES_H

#define CTRL(x)             ((x) & 0x1f)
#define ENTER(x)            ((x==KEY_ENTER)||(x=='\n')||(x=='\r'))
#define ESCAPE              CTRL('[')

#include "main.h"
#include <cursslk.h>

// Utils
int MaxX(void);
int MaxY(void);
int RawMaxY(void);
void MessageBox(const char *msg, ...);
void WarningBox(const char *msg, ...);
bool YesNoBox(const char *msg, ...);
int ChoiceBox(const char *msg, const char *but1, const char *but2, const char *but3=NULL, ...);
std::string InputDialog(const char *title, const char *start=NULL, int max=-1, bool sec=false);
std::string FileDialog(const char *start, const char *info);
std::string MenuDialog(const char *title, const std::vector<std::string> &l, const char *def=NULL);

#include "widgets.h"

class CAboutScreen: public CWidgetBox
{
    CButton *m_pOKButton;
    CTextWindow *m_pTextWin;
    
protected:
    virtual void CreateInit(void);
    virtual bool HandleEvent(CWidgetHandler *p, int type);

public:
    CAboutScreen(CWidgetManager *owner);

    void Run(void) { while (m_pWidgetManager->Run() && !m_bFinished) {}; m_bFinished = false; };
    void UpdateLanguage(void);
};

class CNCursBase: virtual public CMain, public CWidgetWindow
{
    CAboutScreen *m_pAboutScreen;
    
protected:
    CWidgetManager *m_pWidgetManager;

    virtual char *GetPassword(const char *str);
    virtual void MsgBox(const char *str, ...);
    virtual bool YesNoBox(const char *str, ...);
    virtual int ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...);
    virtual void Warn(const char *str, ...);
    
    virtual void CreateInit(void);
    virtual bool HandleKey(chtype ch);
    
public:
    CNCursBase(CWidgetManager *owner);
    virtual ~CNCursBase(void) { };
    
    virtual void UpdateLanguage(void);
};

class CBaseScreen;
class CWelcomeScreen;
class CLicenseScreen;
class CSelectDirScreen;
class CInstallScreen;
class CFinishScreen;
class CCFGScreen;

class CInstaller: public CNCursBase, public CBaseInstall
{
    CButton *m_pCancelButton, *m_pPrevButton, *m_pNextButton;
    std::list<CBaseScreen *> m_InstallScreens;
    std::list<CBaseScreen *>::iterator m_CurrentScreenIt;
    
    CWelcomeScreen *m_pWelcomeScreen;
    CLicenseScreen *m_pLicenseScreen;
    CSelectDirScreen *m_pSelectDirScreen;
    CInstallScreen *m_pInstallScreen;
    CFinishScreen *m_pFinishScreen;
    
    void Prev(void);
    void Next(void);
    
protected:
    virtual void ChangeStatusText(const char *str);
    virtual void AddInstOutput(const std::string &str);
    virtual void SetProgress(int percent);
    virtual void InstallThink(void) { m_pWidgetManager->Run(); };
    
    virtual bool HandleKey(chtype ch);
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    
    virtual bool InitLua(void);
    
public:
    CInstaller(CWidgetManager *owner) : CNCursBase(owner) { };
    
    virtual bool Init(int argc, char **argv);
    virtual void UpdateLanguage(void);
    virtual void Install(void);
    virtual CBaseCFGScreen *CreateCFGScreen(const char *title);
};

class CBaseLuaWidget: public CWidgetWindow
{
    std::string m_szDescription;
    CTextLabel *m_pDescLabel;
    
protected:
    CBaseLuaWidget(CCFGScreen *owner, int y, int x, int maxy, int maxx, const char *desc);
    virtual ~CBaseLuaWidget(void) { };
    
    virtual void CreateInit(void);
    int DescHeight(void) { return (m_pDescLabel) ? m_pDescLabel->height() : 0; };
    
public:
    virtual void UpdateLanguage(void);
};

class CLuaInputField: public CBaseLuaInputField, public CBaseLuaWidget
{
    std::string m_szLabel, m_szValue;
    CInputField *m_pInput;
    CTextLabel *m_pLabel;
    int m_iMax;
    
protected:
    virtual void CreateInit(void);

public:
    CLuaInputField(CCFGScreen *owner, int y, int x, int maxy, int maxx, const char *label, const char *desc, const char *val, int max);
    
    virtual void UpdateLanguage(void);
    virtual const char *GetValue(void) { return m_pInput->GetText().c_str(); };
    virtual void SetSpacing(int percent);
    
    static int CalcHeight(int w, const char *desc);
};

class CLuaCheckbox: public CBaseLuaCheckbox, public CBaseLuaWidget
{
    CCheckbox *m_pCheckbox;
    const std::vector<std::string> m_Options;
    
protected:
    virtual void CreateInit(void);

public:
    CLuaCheckbox(CCFGScreen *owner, int y, int x, int maxy, int maxx, const char *desc,
                 const std::vector<std::string> &l);

    virtual void UpdateLanguage(void);
    virtual bool Enabled(int n) { return m_pCheckbox->IsEnabled(n); };
    virtual void Enable(int n) { m_pCheckbox->EnableBox(n); };

    static int CalcHeight(int w, const char *desc, const std::vector<std::string> &l);
};

class CLuaRadioButton: public CBaseLuaRadioButton, public CBaseLuaWidget
{
    CRadioButton *m_pRadioButton;
    std::vector<std::string> m_Options;
    
protected:
    virtual void CreateInit(void);

public:
    CLuaRadioButton(CCFGScreen *owner, int y, int x, int maxy, int maxx, const char *desc,
                    const std::vector<std::string> &l);

    virtual void UpdateLanguage(void);
    virtual int EnabledButton() { return m_pRadioButton->EnabledButton(); };
    virtual void Enable(int n) { m_pRadioButton->EnableButton(n); };

    static int CalcHeight(int w, const char *desc, const std::vector<std::string> &l);
};

class CLuaDirSelector: public CBaseLuaDirSelector, public CBaseLuaWidget
{
    std::string m_szValue;
    CInputField *m_pDirInput;
    CButton *m_pDirButton;
    
protected:
    virtual void CreateInit(void);
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    
public:
    CLuaDirSelector(CCFGScreen *owner, int y, int x, int maxy, int maxx, const char *desc, const char *val);

    virtual void UpdateLanguage(void);
    virtual const char *GetDir(void) { return m_pDirInput->GetText().c_str(); };
    virtual void SetDir(const char *dir) { m_pDirInput->SetText(dir); };
    
    static int CalcHeight(int w, const char *desc);
};

class CLuaCFGMenu: public CBaseLuaCFGMenu, public CBaseLuaWidget
{
    std::string m_szDesc;
    CMenu *m_pMenu;
    CTextWindow *m_pInfoWindow;
    
    void SetInfo(void);
    
protected:
    virtual void CreateInit(void);
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    
public:
    CLuaCFGMenu(CCFGScreen *owner, int y, int x, int maxy, int maxx, const char *desc);
    
    virtual void UpdateLanguage(void);
    virtual void AddVar(const char *name, const char *desc, const char *val, EVarType type, TOptionsType *l=NULL);
    
    static int CalcHeight(int w, const char *desc);
};

// -------------------------
// Installer screens
// -------------------------

class CBaseScreen: public CWidgetWindow
{
    bool m_bPostInit;
    
    friend class CInstaller;
    
protected:
    CInstaller *m_pInstaller;
    CTextLabel *m_pLabel;

    CBaseScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                int begin_x) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, 'r'),
                               m_bPostInit(true), m_pInstaller(owner) { SetBox(false); erase(); };
                               
    void SetInfo(const char *text);
    
    // This is called when the screen is drawn for the first time. Mainly used if screen needs to init stuff using config
    // variabeles.
    virtual void PostInit(void) { };

public:
    virtual ~CBaseScreen(void) { };

    virtual bool Prev(void) { return true; };
    virtual bool Next(void) { return true; };
    virtual void Activate(void);
    virtual bool CanActivate(void) { return true; };
    virtual void UpdateLanguage(void) { };
};

class CLangScreen: public CBaseScreen
{
    CMenu *m_pLangMenu;
    
protected:
    virtual void CreateInit(void);
    virtual void PostInit(void);

public:
    CLangScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                int begin_x) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x) { };
    
    virtual bool Next(void) { m_pInstaller->m_szCurLang = m_pLangMenu->GetCurrentItemName(); m_pInstaller->UpdateLanguage(); return true; };
    virtual bool CanActivate(void) { return (CBaseScreen::CanActivate() && (m_pInstaller->m_Languages.size() > 1)); };
};

class CWelcomeScreen: public CBaseScreen
{
    CTextWindow *m_pTextWin;
    std::string m_szFileName;

protected:
    virtual bool HandleKey(chtype ch);
    virtual void CreateInit(void);
    virtual void PostInit(void);

public:
    CWelcomeScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                   int begin_x) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x) { };
    
    virtual bool CanActivate(void);
    virtual void UpdateLanguage(void);
};

class CLicenseScreen: public CBaseScreen
{
    CTextWindow *m_pTextWin;
    std::string m_szFileName;

protected:
    virtual bool HandleKey(chtype ch);
    virtual void CreateInit(void);
    virtual void PostInit(void);

public:
    CLicenseScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                   int begin_x) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x) { };

    virtual bool Next(void);
    virtual bool CanActivate(void);
    virtual void UpdateLanguage(void);
};

class CSelectDirScreen: public CBaseScreen
{
    CInputField *m_pFileField;
    CButton *m_pChangeDirButton;
    
protected:
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    virtual void CreateInit(void);
    virtual void PostInit(void);

public:
    CSelectDirScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                     int begin_x) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x) { };

    virtual bool Next(void);
    virtual bool CanActivate(void) { return (m_pInstaller->m_InstallInfo.dest_dir_type == "select"); };
    virtual void UpdateLanguage(void);
};

class CInstallScreen: public CBaseScreen
{
    CProgressbar *m_pProgressbar;
    CTextWindow *m_pTextWin;
    CTextLabel *m_pProgLabel, *m_pStatLabel;
    
protected:
    virtual bool HandleKey(chtype ch);
    virtual void CreateInit(void);
    
public:
    CInstallScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                   int begin_x) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x) { };
    
    void ChangeStatusText(const char *txt);
    void AppendText(const std::string &str) { m_pTextWin->AddText(str); m_pTextWin->refresh(); };
    void SetProgress(int n) { m_pProgressbar->SetCurrent(n); m_pProgressbar->refresh(); };
    
    virtual void Activate(void);
    virtual void UpdateLanguage(void);
};

class CFinishScreen: public CBaseScreen
{
    CTextWindow *m_pTextWin;
    std::string m_szFileName;

protected:
    virtual bool HandleKey(chtype ch);
    virtual void CreateInit(void);
    virtual void PostInit(void);

public:
    CFinishScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                  int begin_x) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x) { };

    virtual bool CanActivate(void);
    virtual void UpdateLanguage(void);
};

class CCFGScreen: public CBaseScreen, public CBaseCFGScreen
{
    std::string m_szTitle;
    int m_iStartY;
    CCFGScreen *m_pNextScreen; 
    std::vector<CBaseLuaWidget *> m_LuaWidgets;
    int m_iLinkedScrNr, m_iLinkedScrMax;
    
    friend class CInstaller;
    
protected:
    virtual bool HandleKey(chtype ch);
    virtual void CreateInit(void);

public:
    CCFGScreen(CInstaller *owner, int nlines, int ncols, int begin_y, int begin_x,
               const std::string &title) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x), m_szTitle(title), m_iStartY(2),
                                           m_pNextScreen(NULL), m_iLinkedScrNr(0), m_iLinkedScrMax(0) { };
    
    virtual CBaseLuaInputField *CreateInputField(const char *label, const char *desc, const char *val, int max);
    virtual CBaseLuaCheckbox *CreateCheckbox(const char *desc, const std::vector<std::string> &l);
    virtual CBaseLuaRadioButton *CreateRadioButton(const char *desc, const std::vector<std::string> &l);
    virtual CBaseLuaDirSelector *CreateDirSelector(const char *desc, const char *val);
    virtual CBaseLuaCFGMenu *CreateCFGMenu(const char *desc);
    
    virtual void Activate(void);
    virtual void UpdateLanguage(void);
    
    // Incase this is a linked screen from a spreaded screen these values will be used to display current and max spreaded screen
    void SetCounter(int n, int max) { m_iLinkedScrNr = n; m_iLinkedScrMax = max; };
};

extern CWidgetManager *pWidgetManager;

#endif
