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

#include "fltk.h"
#include <FL/x.H>
    
int main(int argc, char **argv)
{
    // Init
    if (!MainInit(argc, argv))
    {
        printf("Error: %s\n", GetTranslation("Init failed, aborting"));
        return 1;
    }

    //CreateMainWindow(argv);
    if ((argc > 1) && !strcmp(argv[1], "inst"))
        CInstaller inst(argv);
    else
        CAppManager app(argv);
    
    // Deinit
    MainEnd();
    return 0;
}

void EndProg(bool err)
{
    // HACK: Restore bell volume
    XKeyboardControl XKBControl;
    XKBControl.bell_duration = -1;
    XChangeKeyboardControl(fl_display, KBBellDuration, &XKBControl);

    MainEnd();
    exit((err) ? EXIT_FAILURE : EXIT_SUCCESS);
}

void throwerror(bool dialog, const char *error, ...)
{
    static char txt[1024];
    const char *translated = GetTranslation(error);
    va_list v;
    
    va_start(v, error);
        vsprintf(txt, translated, v);
    va_end(v);

    if (dialog) fl_alert(txt);
    else { fprintf(stderr, GetTranslation("Error: %s"), txt); fprintf(stderr, "\n"); }

    EndProg(true);
}

// -------------------------------------
// FLTK Base screen class
// -------------------------------------

CFLTKBase::CFLTKBase(void)
{
    fl_register_images();
    Fl::scheme("plastic");

    // Create about dialog
    m_pAboutWindow = new Fl_Window(400, 200, "About nixstaller");
    m_pAboutWindow->set_modal();
    m_pAboutWindow->hide();
    m_pAboutWindow->begin();

    Fl_Text_Buffer *pBuffer = new Fl_Text_Buffer;
    pBuffer->loadfile("about");
    
    Fl_Text_Display *pAboutDisp = new Fl_Text_Display(20, 20, 360, 150, "About");
    pAboutDisp->buffer(pBuffer);
    
    m_pAboutOKButton = new Fl_Return_Button((400-80)/2, 170, 80, 25, "OK");
    m_pAboutOKButton->callback(AboutOKCB, this);
    
    m_pAboutWindow->end();

    // HACK: Switch that annoying bell off!
    // UNDONE: Doesn't work on NetBSD yet :(
    XKeyboardControl XKBControl;
    XKBControl.bell_duration = 0;
    XChangeKeyboardControl(fl_display, KBBellDuration, &XKBControl);
}

void CFLTKBase::UpdateLanguage()
{
    // Translations for FL ASK dialogs
    fl_yes = GetTranslation("Yes");
    fl_no = GetTranslation("No");
    fl_ok = GetTranslation("OK");
    fl_cancel = GetTranslation("Cancel");
    fl_close = GetTranslation("Close");

    // Translations for FLTK's File Chooser
    Fl_File_Chooser::add_favorites_label = GetTranslation("Add to Favorites");
    Fl_File_Chooser::all_files_label = GetTranslation("All Files (*)");
    Fl_File_Chooser::custom_filter_label = GetTranslation("Custom Filter");
    Fl_File_Chooser::favorites_label = GetTranslation("Favorites");
    Fl_File_Chooser::filename_label = GetTranslation("Filename:");
    Fl_File_Chooser::filesystems_label = GetTranslation("File Systems");
    Fl_File_Chooser::manage_favorites_label = GetTranslation("Manage Favorites");
    Fl_File_Chooser::new_directory_label = GetTranslation("Enter name of new directory");
    Fl_File_Chooser::new_directory_tooltip = GetTranslation("Create new directory");
    Fl_File_Chooser::show_label = GetTranslation("Show:");
    
    // Update main buttons
    m_pAboutButton->label(GetTranslation("About"));
}

void CFLTKBase::ShowAbout(bool show)
{
    if (show)
    {
        m_pAboutWindow->hotspot(m_pAboutOKButton);
        m_pAboutWindow->take_focus();
        m_pAboutWindow->show();
    }
    else
        m_pAboutWindow->hide();
}
