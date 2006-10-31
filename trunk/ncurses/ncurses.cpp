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

#include "ncurses.h"

NCursesWindow *pRootWin;
CWidgetManager *pWidgetManager;

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    Intro();
    
    // Init ncurses
    
    ::endwin();
    
    // Create root window
    pRootWin = new NCursesWindow(::stdscr);
    NCursesWindow::useColors(); // Use colors when possible
    
    pWidgetManager = new CWidgetManager;
    pWidgetManager->Init();
    
    curs_set(0); // Hide cursor when possible
    
    pRootWin->bkgd(' '|CWidgetWindow::GetColorPair(0, 0)); // No background

    CNCursBase *pInterface;
    //if ((argc > 1) && !strcmp(argv[1], "inst"))
        pInterface = pWidgetManager->AddChild(new CInstaller(pWidgetManager));
    //else
    //pInterface = new CAppManager;
    
    // Init
    if (!pInterface->Init(argc, argv))
    {
        pInterface->ThrowError(false, "Error: Init failed, aborting\n"); // UNDONE
    }

    pWidgetManager->Refresh();
    while (pWidgetManager->Run());
    
    delete pWidgetManager;
    delete pRootWin;
    ::endwin();
    
    pWidgetManager = NULL;
    
    return EXIT_SUCCESS;
}

// -------------------------------------
// About screen class
// -------------------------------------

CAboutScreen::CAboutScreen(CWidgetManager *owner) : CWidgetBox(owner, 20, 50, 0, 0)
{
}

void CAboutScreen::CreateInit()
{
    CWidgetWindow::CreateInit();
    
    SetTitle(GetTranslation("About"));
    
    m_pTextWin = AddChild(new CTextWindow(this, 15, 46, 2, 2, false, false, 'r'));
    m_pTextWin->LoadFile("about");

    m_pOKButton = AddChild(new CButton(this, 1, 10, (m_pTextWin->rely()+m_pTextWin->maxy()+2), (width()-10)/2, GetTranslation("OK"), 'r'));
    Fit(m_pOKButton->rely()+m_pOKButton->maxy()+2);
    
    ActivateChild(m_pTextWin);
    m_pTextWin->BindKeyWidget(m_pOKButton);
}

bool CAboutScreen::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pOKButton)
    {
        m_bFinished = true;
        return true;
    }
    
    return false;
}

void CAboutScreen::UpdateLanguage()
{
    SetTitle(GetTranslation("About"));
    m_pOKButton->SetTitle(GetTranslation("OK"));
}

// -------------------------------------
// NCurses base interface class
// -------------------------------------

CNCursBase::CNCursBase(CWidgetManager *owner) : CWidgetWindow(owner, Min(30, MaxY()-3), Min(60, MaxX()-4), 0, 0), m_pWidgetManager(owner)
{
    m_pAboutScreen = pWidgetManager->AddChild(new CAboutScreen(pWidgetManager));
    m_pAboutScreen->Enable(false);

    // Center & apply
    mvwin(((MaxY() - maxy())/2), ((MaxX() - maxx())/2));
    ::erase();
    pWidgetManager->Refresh();
}

void CNCursBase::CreateInit()
{
    CWidgetWindow::CreateInit();
    
    AddGlobalButton("ESC", "Exit");
    AddGlobalButton("F3", "About");
    AddGlobalButton("ENTER", "Done");
}

char *CNCursBase::GetPassword(const char *str)
{
    return strdup(::InputDialog(str, NULL, -1, true).c_str());
}

void CNCursBase::MsgBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
        vasprintf(&text, str, v);
    va_end(v);
    
    ::MessageBox(text);
    
    free(text);
}

bool CNCursBase::YesNoBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    bool ret = ::YesNoBox(text);
    
    free(text);
    
    return ret;
}

int CNCursBase::ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...)
{
    char *text;
    va_list v;
    
    va_start(v, button3);
    vasprintf(&text, str, v);
    va_end(v);
    
    int ret = ::ChoiceBox(text, button1, button2, button3);
    
    free(text);
    
    return ret;
}

void CNCursBase::WarnBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    ::WarningBox(text);
    
    free(text);
}

bool CNCursBase::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    if (ch == KEY_F(3))
    {
        m_pAboutScreen->Enable(true);
        pWidgetManager->ActivateChild(m_pAboutScreen);
        
        pWidgetManager->Refresh();
        
        m_pAboutScreen->Run();
        
        m_pAboutScreen->Enable(false);
        ::erase(); // Clear whole screen
        pWidgetManager->Refresh();
        return true;
    }
    
    return false;
}

bool CNCursBase::InitLua()
{
    // Overide print function
    m_LuaVM.RegisterFunction(LuaLog, "print", NULL, this);
    
    return CMain::InitLua();
}

void CNCursBase::UpdateLanguage()
{
    CMain::UpdateLanguage();
    
    m_pAboutScreen->UpdateLanguage();
}
