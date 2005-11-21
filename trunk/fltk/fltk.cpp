#include "fltk.h"
#include <FL/x.H>

void CreateMainWindow(char **argv);
void UpdateLanguage(void);
void EndProg(void);

Fl_Window *MainWindow = NULL;
Fl_Wizard *Wizard = NULL;
Fl_Button *pCancelButton = NULL;
Fl_Button *pPrevButton = NULL;
Fl_Button *pNextButton = NULL;

std::list<CBaseScreen *> ScreenList;
bool InstallFiles = false;

void WizCancelCB(Fl_Widget *, void *) { EndProg(-1); };

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
            }
            break;
        }
    }
};

int main(int argc, char **argv)
{
    // Init
    if (!MainInit(argc, argv))
    {
        printf("Error: %s\n", GetTranslation("Init failed, aborting"));
        return 1;
    }

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
    MainWindow = new Fl_Window(MAIN_WINDOW_W, MAIN_WINDOW_H, "Nixstaller");
    
    pCancelButton = new Fl_Button(20, (MAIN_WINDOW_H-30), 120, 25, "Cancel");
    pCancelButton->callback(WizCancelCB, 0);
    pPrevButton = new Fl_Button(MAIN_WINDOW_W-280, (MAIN_WINDOW_H-30), 120, 25, "@<-    Back");
    pPrevButton->callback(WizPrevCB, 0);
    pPrevButton->deactivate(); // First screen, no go back button yet
    pNextButton = new Fl_Button(MAIN_WINDOW_W-140, (MAIN_WINDOW_H-30), 120, 25, "Next    @->");
    pNextButton->callback(WizNextCB, 0);
    
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
    pCancelButton->label(GetTranslation("Cancel"));
    pPrevButton->label(CreateText("@<-    %s", GetTranslation("Back")));
    pNextButton->label(CreateText("%s    @->", GetTranslation("Next")));
    
    // Update all screens
    for(std::list<CBaseScreen *>::iterator p=ScreenList.begin();p!=ScreenList.end();p++) (*p)->UpdateLang();
}

void EndProg(int exitcode)
{
    for(std::list<CBaseScreen *>::iterator p=ScreenList.begin();p!=ScreenList.end();p++) delete *p;
    MainEnd();
    exit(exitcode);
}
