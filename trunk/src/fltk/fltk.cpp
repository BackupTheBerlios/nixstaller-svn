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

#include "fltk.h"
#include "installer.h"
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Window.H>

namespace {
CInstaller *pInterface = NULL;
}

void StartFrontend(int argc, char **argv)
{
    pInterface = new CInstaller;
    pInterface->Init(argc, argv);
    pInterface->Run();
}

void StopFrontend()
{
    delete pInterface;
    pInterface = NULL;
}

void ReportError(const char *msg)
{
    fl_message(msg);
}

#ifndef RELEASE
void debugline(const char *t, ...)
{
    char *txt;
    va_list v;
    
    va_start(v, t);
    vasprintf(&txt, t, v);
    va_end(v);
    
    printf("DEBUG: %s", txt);
    
    free(txt);
}
#endif

