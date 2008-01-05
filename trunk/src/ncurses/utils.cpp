/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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
    extern bool g_bGotGUI;
    
    if (g_bGotGUI)
    {
        try
        {
            MessageBox(msg);
        }
        catch(NCursesException)
        {
            fprintf(stderr, msg);
        }
    }
    else
        fprintf(stderr, msg);
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
    
    int width = std::min(45, MaxX());
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
    
    int width = std::min(45, MaxX());
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
    
    int width = std::min(45, MaxX());
    
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
    
    int width = std::min(65, MaxX());
    
    CChoiceBox *choicebox = pWidgetManager->AddChild(new CChoiceBox(pWidgetManager, MaxY(), width, 0, 0, text, but1, but2, but3));
    int ret = choicebox->Run();
    
    free(text);
    pWidgetManager->RemoveChild(choicebox);
    
    return ret;
}

std::string InputDialog(const char *title, const char *start, int max, bool sec)
{
    int width = std::min(60, MaxX());
    int height = std::min(30, MaxY());
    CInputDialog *textdialog = pWidgetManager->AddChild(new CInputDialog(pWidgetManager, height, width, 0, 0, title, max, sec));
    
    if (start)
        textdialog->SetText(start);
    
    std::string ret = textdialog->Run();
    
    pWidgetManager->RemoveChild(textdialog);
    
    return ret;
}
        
std::string FileDialog(const char *start, const char *info)
{
    int width = std::min(70, MaxX());
    int height = std::min(30, MaxY()-2);
    CFileDialog *filedialog = pWidgetManager->AddChild(new CFileDialog(pWidgetManager, height, width, 0, 0, start, info));
    std::string ret = filedialog->Run();
    
    pWidgetManager->RemoveChild(filedialog);
    
    return ret;
}

