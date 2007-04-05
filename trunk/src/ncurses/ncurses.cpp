/*
    Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.
    
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version. 
    
    This program is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE. See the GNU General Public License for more details. 
    
    You should have received a copy of the GNU General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
    St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "ncurses.h"

NCursesWindow *pRootWin;
CWidgetManager *pWidgetManager;
bool g_bGotGUI;

#if 0
void StartFrontend(int argc, char **argv)
{
    try
    {
        // Init ncurses
        
        ::endwin();
        
        // Create root window
        pRootWin = new NCursesWindow(::stdscr);
        g_bGotGUI = true;
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
        pInterface->Init(argc, argv);
    
        pWidgetManager->Refresh();
        while (pWidgetManager->Run())
            ;
    }
    catch(NCursesException &e)
    {
        // Convert to exception that main() can use...
        throw Exceptions::CExFrontend(CreateText("Ncurses detected the following error:\n%s\n"
                                                 "Note: Your terminal size might be too small", e.message));
    }
}

void StopFrontend()
{
    delete pWidgetManager;
    delete pRootWin;
    ::endwin();
    
    pWidgetManager = NULL;
    g_bGotGUI = false;
}

#else

#include "tui/widget.h"
#include "tui/window.h"
#include "tui/box.h"
#include "tui/label.h"
#include "tui/tui.h"
#include "tui/button.h"
#include "tui/textfield.h"
#include "tui/menu.h"
                                                         
void ReportError(const char *msg)
{
    // UNDONE
    FILE *f=fopen("log.txt", "w");
    fprintf(f, msg);
    fclose(f);
}

void StartFrontend(int argc, char **argv)
{
    NNCurses::TUI.InitNCurses();
    
    NNCurses::CWindow *win = new NNCurses::CWindow();
    win->SetMinWidth(60);
    win->SetMinHeight(5);
    win->SetFColors(COLOR_YELLOW, COLOR_BLUE);
    win->SetDFColors(COLOR_WHITE, COLOR_BLUE);
    
    NNCurses::CBox *vbox = new NNCurses::CBox(NNCurses::CBox::VERTICAL, false);
    vbox->SetBox(false);
    
    NNCurses::CTextField *text = new NNCurses::CTextField(35, 6, false);
    text->LoadFile("about");
    vbox->AddWidget(text);
    
    NNCurses::CMenu *menu = new NNCurses::CMenu(35, 6);
    
    for (short s=0; s<25; s++)
        menu->AddEntry(CreateText("%d", s), CreateText("Menu Line %d", s));
    
    menu->Select("2");
    
    vbox->AddWidget(menu);
    
    NNCurses::CBox *hbox = new NNCurses::CBox(NNCurses::CBox::HORIZONTAL, true, 0);
    hbox->SetMinHeight(1);
    hbox->SetMinWidth(1);
    
    NNCurses::CButton *but = new NNCurses::CButton("OK");
    hbox->StartPack(but, false, false, 1);
    
    but = new NNCurses::CButton("Cancel");
    hbox->StartPack(but, false, false, 1);
    
    but = new NNCurses::CButton("Something between");
    hbox->StartPack(but, false, false, 1);

    vbox->EndPack(hbox, false, false, 1);
    
    win->AddWidget(vbox);
    
    NNCurses::TUI.AddGroup(win, true);
    

//     NNCurses::CBox *hbox = new NNCurses::CBox(NNCurses::CBox::HORIZONTAL);
//     hbox->SetDFColors(COLOR_YELLOW, COLOR_RED);
//     
//     
//     NNCurses::CLabel *label, *l = new NNCurses::CLabel();
//     label = l;
//     label->SetBox(true);
//     label->SetText("ugh");
//     label->SetDFColors(COLOR_YELLOW, COLOR_BLUE);
//     hbox->StartPack(label, false, true, 0);
//     
//     label = new NNCurses::CLabel();
//     label->SetBox(true);
//     label->SetText("hoi");
//     hbox->StartPack(label, true, true, 0);
// 
//     vbox->AddWidget(hbox);
//     
//     label = new NNCurses::CLabel();
//     label->SetBox(true);
//     label->SetText("hoi");
// //     label->SetMinHeight(3);
//     vbox->StartPack(label, true, true, 0);
//     
//     label = new NNCurses::CLabel();
//     label->SetBox(true);
//     label->SetText("hoi");
// //     label->SetMinHeight(3);
//     vbox->StartPack(label, true, true, 0);
//     
//     win->StartPack(vbox, true, true, 0);
// 
//     NNCurses::TUI.Run();
//     sleep (2);
//     
//     l->Enable(false);
//     NNCurses::TUI.Run();
//     sleep (2);
// 
//     l->Enable(true);
    
    while (NNCurses::TUI.Run())
        ;
}

void StopFrontend()
{
    NNCurses::TUI.StopNCurses();
}

#endif

#if 0
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

CNCursBase::CNCursBase(CWidgetManager *owner) : CWidgetWindow(owner, std::min(32, MaxY()-3), std::min(60, MaxX()-4), 0, 0), m_pWidgetManager(owner)
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
    return StrDup(::InputDialog(str, NULL, -1, true).c_str());
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

void CNCursBase::InitLua()
{
    // Overide print function
    m_LuaVM.RegisterFunction(LuaLog, "print", NULL, this);
    CMain::InitLua();
}

void CNCursBase::UpdateLanguage()
{
    CMain::UpdateLanguage();
    
    m_pAboutScreen->UpdateLanguage();
}
#endif
