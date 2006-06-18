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

#include "cursesapp.h"

#include "main.h"
#include "widgets.h"

#define CTRL(x)             ((x) & 0x1f)
#define ENTER(x)            ((x==KEY_ENTER)||(x=='\n')||(x=='\r'))
#define ESCAPE              CTRL('[')

class CAboutScreen: public CWidgetBox
{
    CButton *m_pOKButton;
    CTextWindow *m_pTextWin;
    
protected:
    virtual bool HandleEvent(CWidgetHandler *p, int type);

public:
    CAboutScreen(CWidgetManager *owner);

    void Run(void) { while (m_pWidgetManager->Run() && !m_bFinished) {}; };
};

class CNCursBase: virtual public CMain, public CWidgetWindow
{
    CAboutScreen *m_pAboutScreen;
    
protected:
    virtual char *GetPassword(const char *str);
    virtual void MsgBox(const char *str, ...);
    virtual bool YesNoBox(const char *str, ...);
    virtual int ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...);
    virtual void Warn(const char *str, ...);
    
    virtual bool HandleKey(chtype ch);
    
public:
    CNCursBase(CWidgetManager *owner);
    virtual ~CNCursBase(void) { };
};

class CBaseScreen;

class CInstaller: public CNCursBase, public CBaseInstall
{
    CButton *m_pCancelButton, *m_pPrevButton, *m_pNextButton;
    std::list<CBaseScreen *> m_InstallScreens;
    std::list<CBaseScreen *>::iterator m_CurrentScreenIt;
    
    void Prev(void);
    void Next(void);
    
protected:
    virtual void ChangeStatusText(const char *str, int curstep, int maxsteps) { };
    virtual void AddInstOutput(const std::string &str) { };
    virtual void SetProgress(int percent) { };
    
    virtual bool HandleKey(chtype ch);
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    
public:
    bool m_bInstallFiles;
    
    CInstaller(CWidgetManager *owner) : CNCursBase(owner), m_bInstallFiles(false) { };
    
    virtual bool Init(void);
};

// -------------------------
// Installer screens
// -------------------------

class CBaseScreen: public CWidgetWindow
{
    bool m_bNeedDrawInit;
    
protected:
    CInstaller *m_pInstaller;
    CTextLabel *m_pLabel;

    CBaseScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                int begin_x) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, 'r'),
                               m_bNeedDrawInit(true), m_pInstaller(owner) { SetBox(false); erase(); };
                               
    virtual void DrawInit(void) = 0;

    void SetInfo(const char *text);

public:
    virtual ~CBaseScreen(void) { };

    virtual bool Prev(void) { return true; };
    virtual bool Next(void) { return true; };
    virtual bool Activate(void);
};

class CLangScreen: public CBaseScreen
{
    CMenu *m_pLangMenu;
    
protected:
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    virtual void DrawInit(void);
    
public:
    CLangScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                int begin_x) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x) { };
};

class CWelcomeScreen: public CBaseScreen
{
    CTextWindow *m_pTextWin;

protected:
    virtual bool HandleKey(chtype ch);
    virtual void DrawInit(void);

public:
    CWelcomeScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                   int begin_x) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x) { };
    
    virtual bool Activate(void);
};

class CLicenseScreen: public CBaseScreen
{
    CTextWindow *m_pTextWin;

protected:
    virtual bool HandleKey(chtype ch);
    virtual void DrawInit(void);

public:
    CLicenseScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                   int begin_x) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x) { };

    virtual bool Next(void);
    virtual bool Activate(void);
};

class CSelectDirScreen: public CBaseScreen
{
    CInputField *m_pFileField;
    CButton *m_pChangeDirButton;
    
protected:
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    virtual void DrawInit(void);

public:
    CSelectDirScreen(CInstaller *owner, int nlines, int ncols, int begin_y,
                     int begin_x) : CBaseScreen(owner, nlines, ncols, begin_y, begin_x) { };

    //virtual bool Next(void);
};

extern CWidgetManager *pWidgetManager;

// Utils
int MaxX(void);
int MaxY(void);
void MessageBox(const char *msg, ...);
void WarningBox(const char *msg, ...);
bool YesNoBox(const char *msg, ...);
int ChoiceBox(const char *msg, const char *but1, const char *but2, const char *but3=NULL, ...);
std::string InputDialog(const char *title, const char *start=NULL, int max=-1, bool sec=false);
std::string FileDialog(const char *start, const char *info, bool needw);

#endif
