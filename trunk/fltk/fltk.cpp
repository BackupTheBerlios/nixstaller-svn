#include "fltk.h"

void CreateMainWindow(char **argv);
void UpdateLanguage(void);
void EndProg(void);

Fl_Window *MainWindow = NULL;
Fl_Wizard *Wizard = NULL;
Fl_Button *pPrevButton = NULL;
Fl_Button *pNextButton = NULL;

CLangWidget *pLangWidget = NULL;
CLicenseWidget *pLicenseWidget = NULL;

std::list<CBaseWidget *> WidgetList;

void LangMenuCB(Fl_Widget *w, void *) { InstallInfo.cur_lang = ((Fl_Menu_*)w)->mvalue()->text; };
void LicenseCheckCB(Fl_Widget *w, void *) { (((Fl_Button*)w)->value())?pNextButton->activate():pNextButton->deactivate(); };
void OpenDirSelWinCB(Fl_Widget *w, void *p) { ((CSelectDirWidget *)p)->OpenDirChooser(); };
void WizCancelCB(Fl_Widget *, void *) { EndProg(); };

void WizPrevCB(Fl_Widget *, void *)
{
    for(std::list<CBaseWidget *>::iterator p=WidgetList.begin();p!=WidgetList.end();p++)
    {
        if ((*p)->GetGroup() == Wizard->value())
        {
            if (p == WidgetList.begin()) break;
            (*p)->Prev();
            Wizard->prev();
            (*--p)->Activate();
            break;
        }
    }
}

void WizNextCB(Fl_Widget *, void *)
{
    for(std::list<CBaseWidget *>::iterator p=WidgetList.begin();p!=WidgetList.end();p++)
    {
        if ((*p)->GetGroup() == Wizard->value())
        {
            if (p == WidgetList.end()) break;
            (*p)->Next();
            Wizard->next();
            (*++p)->Activate();
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
    for(std::list<CBaseWidget *>::iterator p=WidgetList.begin();p!=WidgetList.end();p++) delete *p;
    MainEnd();
    return 0;
}

void CreateMainWindow(char **argv)
{
    MainWindow = new Fl_Window(MAIN_WINDOW_W, MAIN_WINDOW_H);
    
    (new Fl_Button(20, (MAIN_WINDOW_H-30), 80, 25, "Cancel"))->callback(WizCancelCB, 0);
    pPrevButton = new Fl_Button(MAIN_WINDOW_W-200, (MAIN_WINDOW_H-30), 80, 25, "@<-    Back");
    pPrevButton->callback(WizPrevCB, 0);
    pPrevButton->deactivate(); // First screen, no go back button yet
    pNextButton = new Fl_Button(MAIN_WINDOW_W-100, (MAIN_WINDOW_H-30), 80, 25, "Next    @->");
    pNextButton->callback(WizNextCB, 0);
    
    Wizard = new Fl_Wizard(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    
    CBaseWidget *widget = pLangWidget = new CLangWidget;
    Fl_Group *group = widget->Create();
    if (group) { Wizard->add(group); WidgetList.push_back(widget); }
    
    widget = new CWelcomeWidget;
    group = widget->Create();
    if (group) { Wizard->add(group); WidgetList.push_back(widget); }

    widget = pLicenseWidget = new CLicenseWidget;
    group = widget->Create();
    if (group) { Wizard->add(group); WidgetList.push_back(widget); }

    widget = new CSelectDirWidget;
    group = widget->Create();
    if (group) { Wizard->add(group); WidgetList.push_back(widget); }

    MainWindow->end();
    MainWindow->show(1, argv);
    Fl::run();
}

void UpdateLanguage()
{
    for(std::list<CBaseWidget *>::iterator p=WidgetList.begin();p!=WidgetList.end();p++) (*p)->UpdateLang();
}

void EndProg()
{
    for(std::list<CBaseWidget *>::iterator p=WidgetList.begin();p!=WidgetList.end();p++) delete *p;
    MainEnd();
    exit(-1);
}
