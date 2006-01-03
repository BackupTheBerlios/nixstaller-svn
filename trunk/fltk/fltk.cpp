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

void CreateMainWindow(char **argv);
void UpdateLanguage(void);
void EndProg(void);

Fl_Window *MainWindow = NULL;
Fl_Wizard *Wizard = NULL;
Fl_Window *pAboutWindow = NULL;
Fl_Button *pAboutOKButton = NULL;
Fl_Button *pAboutButton = NULL;
Fl_Button *pCancelButton = NULL;
Fl_Button *pPrevButton = NULL;
Fl_Button *pNextButton = NULL;

std::list<CBaseScreen *> ScreenList;
bool InstallFiles = false;

void AboutOKCB(Fl_Widget *, void *) { pAboutWindow->hide(); };
        
void WizCancelCB(Fl_Widget *, void *)
{
    if (fl_ask("%s\n%s", GetTranslation("This will abort the installation"), GetTranslation("Are you sure?")))
        EndProg();
}

void WizPrevCB(Fl_Widget *, void *)
{
    for(std::list<CBaseScreen *>::iterator p=ScreenList.begin();p!=ScreenList.end();p++)
    {
        if ((*p)->GetGroup() == Wizard->value())
        {
            if (p == ScreenList.begin()) break;
            if ((*p)->Prev())
            {
                p--;
                Wizard->prev();
                while (!(*p)->Activate() && (p != ScreenList.begin())) { Wizard->prev(); p--; }
            }
            break;
        }
    }
}

void WizNextCB(Fl_Widget *, void *)
{
    for(std::list<CBaseScreen *>::iterator p=ScreenList.begin();p!=ScreenList.end();p++)
    {
        if ((*p)->GetGroup() == Wizard->value())
        {
            if (p == ScreenList.end()) break;
            if ((*p)->Next())
            {
                p++;
                Wizard->next();
                while ((p != ScreenList.end()) && (!(*p)->Activate())) { Wizard->next(); p++; }

                if (p == ScreenList.end())
                    EndProg();
            }
            break;
        }
    }
};

void ShowAboutCB(Fl_Widget *, void *)
{
    pAboutWindow->hotspot(pAboutOKButton);
    pAboutWindow->take_focus();
    pAboutWindow->show();
}
    
    
int main(int argc, char **argv)
{
    // Init
    if (!MainInit(argc, argv))
    {
        printf("Error: %s\n", GetTranslation("Init failed, aborting"));
        return 1;
    }

    fl_register_images();

    CreateMainWindow(argv);
    
    // Deinit
    for(std::list<CBaseScreen *>::iterator p=ScreenList.begin();p!=ScreenList.end();p++) delete *p;
    MainEnd();
    return 0;
}

void CreateMainWindow(char **argv)
{
    #define MCreateWidget(w) { widget = new w; \
                               group = widget->Create(); \
                               if (group) { Wizard->add(group); ScreenList.push_back(widget); } }

    Fl::scheme("plastic");

    // Create about dialog
    pAboutWindow = new Fl_Window(400, 200, "About nixstaller");
    pAboutWindow->set_modal();
    pAboutWindow->hide();
    pAboutWindow->begin();

    Fl_Text_Buffer *pBuffer = new Fl_Text_Buffer;
    pBuffer->loadfile("about");
    
    Fl_Text_Display *pAboutDisp = new Fl_Text_Display(20, 20, 360, 150, "About");
    pAboutDisp->buffer(pBuffer);
    
    pAboutOKButton = new Fl_Button((400-80)/2, 170, 80, 25, "OK");
    pAboutOKButton->callback(AboutOKCB, 0);
    
    pAboutWindow->end();

    if ((InstallInfo.dest_dir_type == DEST_DEFAULT) && !WriteAccess(InstallInfo.dest_dir))
        throwerror(true, CreateText("This installer will install files to the following directory:\n%s\n"
                                    "However you don't have write permissions to this directory\n"
                                    "Please restart the installer as a user who does or as the root user",
                                    InstallInfo.dest_dir.c_str()));

    MainWindow = new Fl_Window(MAIN_WINDOW_W, MAIN_WINDOW_H, "Nixstaller");
    MainWindow->callback(WizCancelCB);

    pCancelButton = new Fl_Button(20, (MAIN_WINDOW_H-30), 120, 25, "Cancel");
    pCancelButton->callback(WizCancelCB, 0);
    pPrevButton = new Fl_Button(MAIN_WINDOW_W-280, (MAIN_WINDOW_H-30), 120, 25, "@<-    Back");
    pPrevButton->callback(WizPrevCB, 0);
    pPrevButton->deactivate(); // First screen, no go back button yet
    pNextButton = new Fl_Button(MAIN_WINDOW_W-140, (MAIN_WINDOW_H-30), 120, 25, "Next    @->");
    pNextButton->callback(WizNextCB, 0);
    pAboutButton = new Fl_Button((MAIN_WINDOW_W-80), 5, 60, 12, "About");
    pAboutButton->labelsize(10);
    pAboutButton->callback(ShowAboutCB, 0);

    Wizard = new Fl_Wizard(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    
    CBaseScreen *widget;
    Fl_Group *group;
    
    MCreateWidget(CLangScreen);
    MCreateWidget(CWelcomeScreen);
    MCreateWidget(CLicenseScreen);
    
    if (InstallInfo.dest_dir_type == DEST_SELECT)
    {
        MCreateWidget(CSelectDirScreen);
    }
    
    MCreateWidget(CSetParamsScreen);
    MCreateWidget(CInstallFilesScreen);
    MCreateWidget(CFinishScreen);
    
    ScreenList.front()->Activate();
    
    // HACK: Switch that annoying bell off!
    XKeyboardControl XKBControl;
    XKBControl.bell_duration = 0;
    XChangeKeyboardControl(fl_display, KBBellDuration, &XKBControl);
    
    MainWindow->end();
    MainWindow->show(1, argv);
    Fl::run();
}

void UpdateLanguage()
{
    // Translations for FL ASK dialogs
    fl_yes = GetTranslation("Yes");
    fl_no = GetTranslation("No");
    fl_ok = GetTranslation("OK");
    fl_cancel = GetTranslation("Cancel");
    
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
    pAboutButton->label(GetTranslation("About"));
    pCancelButton->label(GetTranslation("Cancel"));
    pPrevButton->label(CreateText("@<-    %s", GetTranslation("Back")));
    pNextButton->label(CreateText("%s    @->", GetTranslation("Next")));
    
    // Update all screens
    for(std::list<CBaseScreen *>::iterator p=ScreenList.begin();p!=ScreenList.end();p++) (*p)->UpdateLang();
}

void EndProg()
{
    // HACK: Restore bell volume
    XKeyboardControl XKBControl;
    XKBControl.bell_duration = -1;
    XChangeKeyboardControl(fl_display, KBBellDuration, &XKBControl);

    for(std::list<CBaseScreen *>::iterator p=ScreenList.begin();p!=ScreenList.end();p++) delete *p;
    MainEnd();
    exit(EXIT_SUCCESS);
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
    
    // HACK: Restore bell volume
    XKeyboardControl XKBControl;
    XKBControl.bell_duration = -1;
    XChangeKeyboardControl(fl_display, KBBellDuration, &XKBControl);

    for(std::list<CBaseScreen *>::iterator p=ScreenList.begin();p!=ScreenList.end();p++) delete *p;
    MainEnd();
    exit(EXIT_FAILURE);
}
