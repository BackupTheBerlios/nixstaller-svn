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

void EndProg(bool err)
{
    extern NCursesWindow *pRootWin;
    
    delete pWidgetManager;
    delete pRootWin;
    ::endwin();
    
    pWidgetManager = NULL;
    
    debugline("EndProg");
    exit((err) ? EXIT_FAILURE : EXIT_SUCCESS);
}

int MaxX()
{
    static int y, x;
    getmaxyx(stdscr, y, x);
    return x;
}

int MaxY()
{
    static int y, x;
    getmaxyx(stdscr, y, x);
    return y - 2; // 2 lines for buttonbar
}

int RawMaxY()
{
    static int y, x;
    getmaxyx(stdscr, y, x);
    return y;
}

void MessageBox(const char *msg, ...)
{
    char *text;
    va_list v;
    
    va_start(v, msg);
    vasprintf(&text, msg, v);
    va_end(v);
    
    int width = Min(35, MaxX());
    CMessageBox *msgbox = pWidgetManager->AddChild(new CMessageBox(pWidgetManager, MaxY(), width, 0, 0, text));
    msgbox->Run();
    
    free(text);
    pWidgetManager->RemoveChild(msgbox);
}

void WarningBox(const char *msg, ...)
{
    char *text;
    va_list v;
    
    va_start(v, msg);
    vasprintf(&text, msg, v);
    va_end(v);
    
    int width = Min(35, MaxX());
    CWarningBox *warnbox = pWidgetManager->AddChild(new CWarningBox(pWidgetManager, MaxY(), width, 0, 0, text));
    warnbox->Run();
    
    free(text);
    pWidgetManager->RemoveChild(warnbox);
}

bool YesNoBox(const char *msg, ...)
{
    char *text;
    va_list v;
    
    va_start(v, msg);
    vasprintf(&text, msg, v);
    va_end(v);
    
    int width = Min(40, MaxX());
    
    CYesNoBox *yesnobox = pWidgetManager->AddChild(new CYesNoBox(pWidgetManager, MaxY(), width, 0, 0, text));
    bool ret = yesnobox->Run();
    
    free(text);
    pWidgetManager->RemoveChild(yesnobox);
    
    return ret;
}

int ChoiceBox(const char *msg, const char *but1, const char *but2, const char *but3, ...)
{
    char *text;
    va_list v;
    
    va_start(v, but3);
    vasprintf(&text, msg, v);
    va_end(v);
    
    int width = Min(65, MaxX());
    
    CChoiceBox *choicebox = pWidgetManager->AddChild(new CChoiceBox(pWidgetManager, MaxY(), width, 0, 0, text, but1, but2, but3));
    int ret = choicebox->Run();
    
    free(text);
    pWidgetManager->RemoveChild(choicebox);
    
    return ret;
}

std::string InputDialog(const char *title, const char *start, int max, bool sec)
{
    int width = Min(60, MaxX());
    int height = Min(30, MaxY());
    CInputDialog *textdialog = pWidgetManager->AddChild(new CInputDialog(pWidgetManager, height, width, 0, 0, title, max, sec));
    
    if (start)
        textdialog->SetText(start);
    
    std::string ret = textdialog->Run();
    
    pWidgetManager->RemoveChild(textdialog);
    
    return ret;
}
        
std::string FileDialog(const char *start, const char *info)
{
    int width = Min(70, MaxX());
    int height = Min(30, MaxY()-2);
    CFileDialog *filedialog = pWidgetManager->AddChild(new CFileDialog(pWidgetManager, height, width, 0, 0, start, info));
    std::string ret = filedialog->Run();
    
    pWidgetManager->RemoveChild(filedialog);
    
    return ret;
}

std::string MenuDialog(const char *title, const std::list<std::string> &l, const char *def)
{
    int width = Min(50, MaxX());
    int height = Min(15, MaxY());
    CMenuDialog *dialog = pWidgetManager->AddChild(new CMenuDialog(pWidgetManager, height, width, 0, 0, title));
    
    for (std::list<std::string>::const_iterator it=l.begin(); it!=l.end(); it++)
        dialog->AddItem(*it);
    
    if (def && *def)
        dialog->SetItem(def);
    
    dialog->refresh();
    std::string ret = dialog->Run();
    
    pWidgetManager->RemoveChild(dialog);
    
    return ret;
}
