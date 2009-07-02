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

#include <locale.h>
#include <libgen.h>
#include <stdlib.h>

#include "main/main.h"
#include "main/lua/lua.h"
#include "main/lua/luatable.h"
#include "curl.h"
#include "run.h"
#include "suterm.h"
#include "unattinstall.h"
#include "utils.h"

namespace {
    bool g_RunScript = false, g_RunUnattended = false, g_Error = false;
}

bool HadError()
{
    return g_Error;
}

// Besides main(), other functions may want to call this incase they cannot throw an exception
void Quit(int ret)
{
    g_Error = (ret != 0);
    
    if (!g_RunScript && !g_RunUnattended)
        StopFrontend();
    
#ifndef RELEASE
    NLua::StackDump("Clean stack?\n");
#endif
    
    FreeStrings();
    FreeTranslations();

    curl_global_cleanup();
    exit(ret);
}

// Besides main(), other functions may want to call this incase they wants to stop an exception flow (ie GTK frontend)
void HandleError(void)
{
    try
    {
        throw;
    }
    catch(Exceptions::CExUser &e)
    {
        debugline("User cancel\n");
    }
    catch(Exceptions::CException &e)
    {
        fflush(stdout);
        fflush(stderr);
        
        if (g_RunScript || g_RunUnattended)
            fprintf(stderr, "%s: %s\n", GetTranslation("Error"), GetTranslation(e.what())); // No specific way to complain, just use stderr
        else
            ReportError(CreateText("%s: %s", GetTranslation("Error"), GetTranslation(e.what())));
    }
    
    Quit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    unlink("frontendstarted");
    
    setlocale(LC_ALL, "");

    PrintIntro();

    std::string surunnerpath = DirName(argv[0]);
    if (!FileExists(JoinPath(surunnerpath, "surunner")))
        surunnerpath += "/.."; // HACK: for static surunner bins
    CSuTerm::SetRunnerPath(surunnerpath);
    
    curl_global_init(CURL_GLOBAL_ALL);

    if (argc > 1)
    {
        if (!strcmp(argv[1], "run"))
            g_RunScript = true;
        else if (!strcmp(argv[1], "unattended"))
            g_RunUnattended = true;
    }

    try
    {
        if (g_RunScript)
        {
            CLuaRunner lr;
            lr.Init(argc, argv);
        }
        else if (g_RunUnattended)
        {
            CBaseUnattInstall ui;
            ui.Init(argc, argv);
        }
        else
            StartFrontend(argc, argv);
    }
    catch(Exceptions::CException &e)
    {
        HandleError();
        // Handle error calls exit()
    }
    
    Quit(EXIT_SUCCESS);
    return 0; // Never reached
}

// -------------------------------------
// Lua Runner Class
// -------------------------------------

void CLuaRunner::Init(int argc, char **argv)
{
    CMain::Init(argc, argv);
    
    NLua::CLuaTable tab("args");
    bool startluaargs = false;
    int tabind = 1;
    const char *run = NULL;
    
    for (int a=1; a<argc; a++)
    {
        if (!startluaargs)
        {
            if (!strcmp(argv[a], "--"))
                startluaargs = true;
            else if (!strcmp(argv[a], "-n"))
            {
                a++;
                if (a < argc)
                {
                    std::string d = argv[a];
                    MakeAbsolute(d);
                    NLua::LuaSet(d, "nixstdir", "internal");
                }
            }
            else if (!strcmp(argv[a], "-c"))
            {
                a++;
                if (a < argc)
                    NLua::LuaSet(argv[a], "callscript", "internal");
            }
            else if (!strcmp(argv[a], "-e"))
            {
                a++;
                if (a < argc)
                    run = argv[a];
            }
        }
        else
        {
            tab[tabind] << argv[a];
            tabind++;
        }
    }

    tab.Close();
    NLua::LoadFile(run);
}
