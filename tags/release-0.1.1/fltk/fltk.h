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
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Hold_Browser.H>

#define MAIN_WINDOW_W 600
#define MAIN_WINDOW_H 400

void UpdateLanguage(void);

extern Fl_Button *pCancelButton;
extern Fl_Button *pPrevButton;
extern Fl_Button *pNextButton;
extern bool InstallFiles;

class CBaseScreen
{
protected:
    Fl_Group *m_pGroup;
    
public:
    CBaseScreen(void) : m_pGroup(NULL) { };
    virtual ~CBaseScreen(void) { };
    
    virtual Fl_Group *Create(void) = NULL;
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
    virtual Fl_Group *Create(void);
    virtual bool Next(void);
    virtual bool Activate(void);
    
    static void LangMenuCB(Fl_Widget *w, void *) { InstallInfo.cur_lang = ((Fl_Menu_*)w)->mvalue()->text; };
};

class CWelcomeScreen: public CBaseScreen
{
    Fl_Shared_Image *m_pImage;
    Fl_Box *m_pImageBox;
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
    Fl_Box *m_pBoxTitle;
    Fl_Input *m_pParamInput;
    Fl_Choice *m_pValChoiceMenu;
    Fl_Hold_Browser *m_pChoiceBrowser;
    Fl_Multiline_Output *m_pDescriptionOutput;
    Fl_Output *m_pDefOutput;
    Fl_File_Chooser *m_pDirChooser;
    Fl_Button *m_pSelDirButton;
    Fl_Output *m_pSelDirInput;
    param_entry_s *m_pCurrentParamEntry;
    
public:
    CSetParamsScreen(void) : CBaseScreen(), m_pDirChooser(NULL), m_pCurrentParamEntry(NULL) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Activate(void);
    //virtual bool Next(void);
    
    void SetInput(const char *txt, command_entry_s *pCommandEntry);
    void SetValue(const std::string &str);
    void OpenDirChooser(void);
    
    static void ParamBrowserCB(Fl_Widget *w, void *p);
    static void ValChoiceMenuCB(Fl_Widget *w, void *p);
    static void ParamInputCB(Fl_Widget *w, void *p);
    static void OpenDirSelWinCB(Fl_Widget *w, void *p) { ((CSetParamsScreen *)p)->OpenDirChooser(); };
};

class CInstallFilesScreen: public CBaseScreen
{
protected:
    Fl_Progress *m_pProgress;
    Fl_Text_Buffer *m_pBuffer;
    Fl_Text_Display *m_pDisplay;
    
    Fl_Window *m_pAskPassWindow;
    Fl_Box *m_pAskPassBox;
    Fl_Secret_Input *m_pAskPassInput;
    Fl_Return_Button *m_pAskPassOKButton;
    Fl_Button *m_pAskPassCancelButton;
    
    LIBSU::CLibSU m_SUHandler;
    char *m_szPassword;
    
    void ClearPassword(void);
    
public:
    CInstallFilesScreen(void) : CBaseScreen(), m_szPassword(NULL) { };
    virtual ~CInstallFilesScreen(void) { CleanPasswdString(m_szPassword); };
    
    virtual Fl_Group *Create(void);
    virtual bool Activate(void);
    virtual void UpdateLang(void);
    virtual void Install(void);
    
    void AppendText(const char *txt);
    void AppendText(const std::string &txt) { AppendText(txt.c_str()); };
    void ChangeStatusText(const char *txt, int n);
    void SetProgress(int percent) { m_pProgress->value(percent); };
    void SetPassword(bool unset);
    
    static void stat_inst(void *p) { if (InstallFiles) ((CInstallFilesScreen *)p)->Install(); };
    static void SUUpdateProgress(int percent, void *p);
    static void SUUpdateText(const std::string &str, void *p);
    static void SUOutputHandler(const char *msg, void *p) { ((CInstallFilesScreen *)p)->AppendText(msg); Fl::flush(); };
    static void AskPassOKButtonCB(Fl_Widget *w, void *p);
    static void AskPassCancelButtonCB(Fl_Widget *w, void *p);
};

class CFinishScreen: public CBaseScreen
{
    Fl_Text_Display *m_pDisplay;
    Fl_Text_Buffer *m_pBuffer;
    bool m_bHasText;
    
public:
    CFinishScreen(void) : CBaseScreen(), m_bHasText(false) { };
    
    virtual Fl_Group *Create(void);
    virtual void UpdateLang(void);
    virtual bool Activate(void);
};

#endif
