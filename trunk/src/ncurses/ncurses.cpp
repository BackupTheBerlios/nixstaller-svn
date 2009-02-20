/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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
#include "tui/tui.h"
#include "tui/dialog.h"

namespace {
bool g_bGotGUI = false;
}

#ifndef RELEASE
void debugline(const char *t, ...)
{
    char *txt;
    va_list v;
    
    va_start(v, t);
    vasprintf(&txt, t, v);
    va_end(v);
    
    static FILE *f=fopen("/tmp/nixstlog.txt", "w");
    if (f) { fprintf(f, txt); fflush(f); }
    
    free(txt);
}
#endif

void ReportError(const char *msg)
{
    if (g_bGotGUI)
    {
        try
        {
            NNCurses::MessageBox(msg);
        }
        catch(Exceptions::CExFrontend &)
        {
        }
        catch(Exceptions::CExOverflow &)
        {
        }
    }
    
    StopFrontend();
    fprintf(stderr, msg);
}

void StartFrontend(int argc, char **argv)
{
    NNCurses::TUI.InitNCurses();
    
    g_bGotGUI = true;
    
    CInstaller *interface = new CInstaller;
    interface->SetFColors(COLOR_GREEN, COLOR_BLUE);
    interface->SetDFColors(COLOR_WHITE, COLOR_BLUE);
    interface->SetMinWidth(60);
    interface->SetMinHeight(15);
    
    NNCurses::TUI.AddGroup(interface, true);
    interface->Init(argc, argv);

    interface->Run();
}

void StopFrontend()
{
    g_bGotGUI = false;
    NNCurses::TUI.StopNCurses();
}
