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

static CNCursScreen MainScreen;
CWidgetManager WidgetManager;

void EndProg(bool err)
{
//    delete pInterface;
    debugline("EndProg");
    exit((err) ? EXIT_FAILURE : EXIT_SUCCESS);
}

// -------------------------------------
// NCurses screen class
// -------------------------------------

void CNCursScreen::init(bool bColors)
{
    NCursesApplication::init(bColors);
    WidgetManager.Init();
}

void CNCursScreen::init_labels(Soft_Label_Key_Set& S) const
{
    // UNDONE?
}

void CNCursScreen::title()
{
    const char *title = "Nixstaller";
    const int len = strlen(title);

    titleWindow->bkgd(' '|CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_GREEN)|A_BOLD);
    titleWindow->addstr(0, (titleWindow->cols() - len)/2, title);
    titleWindow->noutrefresh();
}

void CNCursScreen::handleArgs(int argc, char* argv[])
{
    // Choose between installer or appmanager here
}

int CNCursScreen::run()
{
    curs_set(0);
    
    CNCursBase *p = new CInstaller;
    //p->Init();
    
    Root_Window->bkgd(' '|CWidgetWindow::GetColorPair(0, 0));
    
    /*int choice = ChoiceBox("Please click some button", "button 1", "button 2", "button 3");
    std::string txt = InputDialog("Please type something", "Hello", 10, false);
    std::string dir = FileDialog("/", "<C>Select a directory please", false);
    MessageBox("You choose: %d\nYou typed: %s\nYou selected dir: %s", choice, txt.c_str(), dir.c_str());*/
    
    // Init installer/appmanager
    
    // Run installer/appmanager
    WidgetManager.Refresh();
    while (WidgetManager.Run());
    
    return EXIT_SUCCESS;
}

// -------------------------------------
// About screen class
// -------------------------------------

CAboutScreen::CAboutScreen(CWidgetManager *owner) : CWidgetBox(owner, 20, 50, 0, 0)
{
    SetTitle("About");
    
    m_pTextWin = new CTextWindow(this, 15, 46, 2, 2, false, false, 'r');
    m_pTextWin->LoadFile("about");

    m_pOKButton = new CButton(this, 1, 10, (m_pTextWin->rely()+m_pTextWin->maxy()+2), (width()-10)/2, "OK", 'r');
    Fit(m_pOKButton->rely()+m_pOKButton->maxy()+2);
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

// -------------------------------------
// NCurses base interface class
// -------------------------------------

CNCursBase::CNCursBase() : CWidgetWindow(&WidgetManager, MaxY()-4, MaxX()-4, 2, 2)
{
    m_pAboutScreen = new CAboutScreen(&WidgetManager);
    m_pAboutScreen->Enable(false);
}

char *CNCursBase::GetPassword(const char *str)
{
    std::string pass = ::InputDialog(str, NULL, -1, true);
    return MakeCString(pass);
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

void CNCursBase::Warn(const char *str, ...)
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
        WidgetManager.ActivateChild(m_pAboutScreen);
        
        WidgetManager.Refresh();
        
        m_pAboutScreen->Run();
        
        m_pAboutScreen->Enable(false);
        ::erase(); // Clear whole screen
        WidgetManager.Refresh();
        
        return true;
    }
    
    return false;
}
