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
    exit((err) ? EXIT_FAILURE : EXIT_SUCCESS);
}

void CNCursScreen::init_labels(Soft_Label_Key_Set& S) const
{
    // UNDONE
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
    CFileDialog *f = new CFileDialog(&WidgetManager, 18, 70, 2, 2, "/", "<C>Select a directory please", false);
    WidgetManager.Refresh();
    while(WidgetManager.Run());
    YesNoBox("O Rly????????????????????????????????????????????????????????????????????????????????????????\nYes Rly\nk Wai?\n\nblah\n\n\n\ndus\n\n\narg");
    // Init installer/appmanager
    
    // Run installer/appmanager
    
    return EXIT_SUCCESS;
}

bool menucb(CWidgetHandler *, int n, int)
{
    debugline("menu: %d", n);
    beep();
}

void CNCursBase::MsgBox(const char *str, ...)
{
    char *text;
    CWidgetManager Man;
    CWidgetWindow *panel = new CWidgetWindow(&Man, 18, 40, 2, 4, true);
    
    va_list v;
    
    va_start(v, str);
        vasprintf(&text, str, v);
    va_end(v);
    
    /*
    panel->boldframe("Message");
    panel->bkgd(' '|MainScreen.dialog_backgrounds());
    panel->printw(1, 1, str);*/
    
    CButton *but = new CButton(panel, 1, 8, 1, 3, "hah", 'r');
    CButton *but2 = new CButton(panel, 1, 8, 5, 3, "hah", 'r');
    CInputField *in = new CInputField(panel, 3, 12, 5, 15, 'r');
    CTextWindow *twin = new CTextWindow(panel, 8, 15, 8, 2, false, false, 'r', true);
    CMenu *menu = new CMenu(panel, 8, 15, 8, 20, 'r');
    in->SetText("har!");
    
    //m_pDummyPanel->refresh();
    //panel->refresh();
/*    but->refresh();
    but2->refresh();
    in->refresh();
    twin->refresh();*/
    
    twin->AddText("<C>centere\n");
    
    for (int i=0;i<51;i++)
    {
        twin->AddText(CreateText("%d <C><col=2>dfs fds</col> sfd <rev>sd f</rev> fsd fds tt\n", i));
        menu->AddItem(CreateText("<C>menu item %d\n", i), menucb, 0);
    }

    //twin->refresh();
    //menu->refresh();

    panel->refresh();
    
    Man.Run();
    delete panel;
    delete but;
    delete but2;
    delete in;
    delete twin;
    delete menu;
    
    //m_pDummyPanel->refresh();
    
    free(text);
}

