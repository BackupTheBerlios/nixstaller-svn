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
#include "installer.h"
#include "main/lua/lua.h"
#include "main/lua/luafunc.h"
#include "tui/tui.h"
#include "tui/widget.h"
#include "tui/window.h"
#include "tui/box.h"
#include "tui/label.h"
#include "tui/button.h"
#include "tui/textfield.h"
#include "tui/menu.h"
#include "tui/inputfield.h"
#include "tui/progressbar.h"
#include "tui/dialog.h"
#include "tui/radiobutton.h"

#ifndef RELEASE
void debugline(const char *t, ...)
{
    char *txt;
    va_list v;
    
    va_start(v, t);
    vasprintf(&txt, t, v);
    va_end(v);
    
    /*bool plain = isendwin();
    if (!plain)
    endwin(); // Disable ncurses mode
    
    printf("DEBUG: %s", txt);
    FILE *f=fopen("log.txt", "a"); if (f) { fprintf(f, txt);fclose(f); }
    fflush(stdout);
    
    if (!plain)
    refresh(); // Reenable ncurses mode*/
    
    static FILE *f=fopen("log.txt", "w");
    if (f) { fprintf(f, txt); fflush(f); }
    
    free(txt);
}
#endif

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
    
    CInstaller *interface = new CInstaller;
    interface->SetFColors(COLOR_GREEN, COLOR_BLUE);
    interface->SetDFColors(COLOR_WHITE, COLOR_BLUE);
    interface->SetMinWidth(60);
    interface->SetMinHeight(15);
    
    interface->Init(argc, argv);
    
    NNCurses::TUI.AddGroup(interface, true);
    
    interface->Run();
}

void StopFrontend()
{
    NNCurses::TUI.StopNCurses();
}

// -------------------------------------
// Base NCurses Frontend Class
// -------------------------------------

char *CNCursBase::GetPassword(const char *str)
{
    return StrDup(NNCurses::InputBox(str, "", 0, '*').c_str());
}

void CNCursBase::ShowAbout()
{
    NNCurses::TColorPair fc(COLOR_GREEN, COLOR_BLUE), dfc(COLOR_WHITE, COLOR_BLUE);
    NNCurses::CDialog *dialog = NNCurses::CreateBaseDialog(fc, dfc, 25, 0);
    
    NNCurses::CTextField *text = new NNCurses::CTextField(30, 12, false);
    text->LoadFile("about");
    dialog->AddWidget(text);
    
    dialog->AddButton(new NNCurses::CButton(GetTranslation("OK")));
    
    NNCurses::TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    delete dialog;
}

void CNCursBase::MsgBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    NNCurses::MessageBox(text);
    
    free(text);
}

bool CNCursBase::YesNoBox(const char *str, ...)
{
    char *text;
    va_list v;
    
    va_start(v, str);
    vasprintf(&text, str, v);
    va_end(v);
    
    bool ret = NNCurses::YesNoBox(text);
    
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
    
    NNCurses::TColorPair fc(COLOR_GREEN, COLOR_BLUE), dfc(COLOR_WHITE, COLOR_BLUE);
    NNCurses::CDialog *dialog = NNCurses::CreateBaseDialog(fc, dfc, 35, 0, text);
        
    NNCurses::CButton *buttons[3];
    
    dialog->AddButton(buttons[0] = new NNCurses::CButton(button1));
    dialog->AddButton(buttons[1] = new NNCurses::CButton(button2));
    
    if (button3)
        dialog->AddButton(buttons[2] = new NNCurses::CButton(button3));
    
    NNCurses::TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    int ret = -1;
    for (int i=0; i<3; i++)
    {
        if (dialog->ActivatedWidget() == buttons[i])
        {
            ret = i;
            break;
        }
    }
    
    delete dialog;
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
    
    NNCurses::WarningBox(text);
    
    free(text);
}

void CNCursBase::InitLua()
{
    // Overide print function
    NLua::RegisterFunction(LuaLog, "print", NULL, this);
    CMain::InitLua();
}

bool CNCursBase::CoreHandleKey(chtype key)
{
    if (NNCurses::CWindow::CoreHandleKey(key))
        return true;
    
    if (key == KEY_F(2))
    {
        ShowAbout();
        return true;
    }
    
    return false;
}

void CNCursBase::Run()
{
    while (NNCurses::TUI.Run())
        ;
}
