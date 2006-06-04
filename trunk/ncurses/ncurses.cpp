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

void CNCursScreen::init_labels(Soft_Label_Key_Set& S) const
{
    // UNDONE?
}

void CNCursScreen::title()
{
    const char *title = "Nixstaller";
    const int len = strlen(title);

    titleWindow->bkgd(screen_titles());
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
    
    CNCursBase p;
    p.Init();
    //p.MsgBox("hi\n");
    
    Root_Window->bkgd(' '|CWidgetWindow::GetColorPair(0, 0));
    
    WidgetManager.Init();
    //CFileDialog *f = new CFileDialog(&WidgetManager, 18, 70, 2, 2, "/", "<C>Select a directory please", false);
    std::string dir = FileDialog("/", "<C>Select a directory please", false);
    if (!dir.empty())
        MessageBox("You selected: %s", dir.c_str());
    else
        MessageBox("Nothing");
    
    // Init installer/appmanager
    
    // Run installer/appmanager
    
    return EXIT_SUCCESS;
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
