#include "fltk.h"

void CreateMainWindow(char **argv);
void UpdateLanguage(void);
void EndProg(void);

Fl_Window *MainWindow = NULL;
Fl_Wizard *Wizard = NULL;
Fl_Button *pPrevButton = NULL;
Fl_Button *pNextButton = NULL;

CLangScreen *pLangScreen = NULL;
CLicenseScreen *pLicenseScreen = NULL;
CInstallFilesScreen *pInstallFilesScreen = NULL;

std::list<CBaseScreen *> ScreenList;
bool InstallFiles = false;

void LangMenuCB(Fl_Widget *w, void *) { InstallInfo.cur_lang = ((Fl_Menu_*)w)->mvalue()->text; };
void LicenseCheckCB(Fl_Widget *w, void *) { (((Fl_Button*)w)->value())?pNextButton->activate():pNextButton->deactivate(); };
void OpenDirSelWinCB(Fl_Widget *w, void *p) { ((CSelectDirScreen *)p)->OpenDirChooser(); };
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
                Wizard->prev();
                (*--p)->Activate();
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
                Wizard->next();
                (*++p)->Activate();
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
    Fl::scheme("plastic");
    MainWindow = new Fl_Window(MAIN_WINDOW_W, MAIN_WINDOW_H);
    
    (new Fl_Button(20, (MAIN_WINDOW_H-30), 80, 25, "Cancel"))->callback(WizCancelCB, 0);
    pPrevButton = new Fl_Button(MAIN_WINDOW_W-200, (MAIN_WINDOW_H-30), 80, 25, "@<-    Back");
    pPrevButton->callback(WizPrevCB, 0);
    pPrevButton->deactivate(); // First screen, no go back button yet
    pNextButton = new Fl_Button(MAIN_WINDOW_W-100, (MAIN_WINDOW_H-30), 80, 25, "Next    @->");
    pNextButton->callback(WizNextCB, 0);
    
    Wizard = new Fl_Wizard(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    
    CBaseScreen *widget = pLangScreen = new CLangScreen;
    Fl_Group *group = widget->Create();
    if (group) { Wizard->add(group); ScreenList.push_back(widget); }
    
    widget = new CWelcomeScreen;
    group = widget->Create();
    if (group) { Wizard->add(group); ScreenList.push_back(widget); }

    widget = pLicenseScreen = new CLicenseScreen;
    group = widget->Create();
    if (group) { Wizard->add(group); ScreenList.push_back(widget); }

    widget = new CSelectDirScreen;
    group = widget->Create();
    if (group) { Wizard->add(group); ScreenList.push_back(widget); }

    widget = pInstallFilesScreen = new CInstallFilesScreen;
    group = widget->Create();
    Fl::add_idle(CInstallFilesScreen::stat_inst, widget);
    if (group) { Wizard->add(group); ScreenList.push_back(widget); }

    MainWindow->end();
    MainWindow->show(1, argv);
    Fl::run();
}

void UpdateLanguage()
{
    for(std::list<CBaseScreen *>::iterator p=ScreenList.begin();p!=ScreenList.end();p++) (*p)->UpdateLang();
}

void EndProg(int exitcode)
{
    for(std::list<CBaseScreen *>::iterator p=ScreenList.begin();p!=ScreenList.end();p++) delete *p;
    MainEnd();
    exit(exitcode);
}
