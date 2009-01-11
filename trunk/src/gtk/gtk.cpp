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

#include <fstream>
#include "gtk.h"
#include "installer.h"

namespace {
CInstaller *pInterface = NULL;
};

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

void StartFrontend(int argc, char **argv)
{
    if (gtk_init_check(&argc, &argv) == FALSE)
        throw Exceptions::CExFrontend("Failed to initialize GTK");
    
    pInterface = new CInstaller();
    pInterface->Init(argc, argv);
    pInterface->Run();
}

void StopFrontend()
{
    // As this function may be called from the interface's destructor, make sure pInterface is NULL
    // before deleting it.
    CInstaller *inter = pInterface;
    pInterface = NULL;
    delete inter;
}

void ReportError(const char *msg)
{
    MessageBox(GTK_MESSAGE_ERROR, msg);
}
