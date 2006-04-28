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
    Root_Window->setcolor(7);
    Root_Window->setpalette(COLOR_WHITE, COLOR_BLACK);
    
    CNCursBase p;
    p.Init();
    p.MsgBox("hi\n");
    
    // Init installer/appmanager
    
    // Run installer/appmanager
    
    return EXIT_SUCCESS;
}

void CNCursBase::MsgBox(const char *str, ...)
{
    char *text;
    CWidgetManager Man;
    CWidgetPanel *panel = new CWidgetPanel(&Man, 18,40,2,4);
    
    va_list v;
    
    va_start(v, str);
        vasprintf(&text, str, v);
    va_end(v);
    
    panel->boldframe("Message");
    panel->bkgd(' '|MainScreen.dialog_backgrounds());
    panel->printw(1, 1, str);
        
    CButton *but = new CButton(panel, 1, 8, 1, 3, "hah", NULL, 'r');
    CButton *but2 = new CButton(panel, 1, 8, 5, 3, "hah", NULL, 'r');
    //CWidgetWindow *pwin = new CWidgetWindow(panel, 8, 15, 8, 2, 'r');
    //CWidgetPad *pad = new CWidgetPad(pwin, 40, 40);
    CTextWindow *twin = new CTextWindow(panel, 8, 15, 8, 2, false, 'r');
//    CScrollbar *scroll = new CScrollbar(panel, 10, 1, 1, 18, 0, 100, true, 'r');
    
    std::string nrs;
    for (int i=0;i<25;i++) twin->AddText(CreateText("%d hkhi upo hity vuoiup uy[p ]o nkuo\n", i));
    //twin->SetText(nrs);
    m_pDummyPanel->refresh();
    but->refresh();
    but2->refresh();
    twin->refresh();
    
/*    for (int i=0;i<40;i++)
        pad->printw("%d h0 h0 h0\n", i);
    pad->refresh();*/
    
  //  scroll->SetCurrent(100);
   // scroll->Scroll(-1);
   // scroll->refresh();
    
    Man.Run();
    
    delete panel;
    delete but;
    delete but2;
    //delete pwin;
    //delete pad;
    delete twin;
    
    m_pDummyPanel->refresh();
    
    sleep(3);
    free(text);
}

