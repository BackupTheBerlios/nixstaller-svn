#include "fltk.h"

void CreateMainWindow(char **argv);
void UpdateLanguage(void);
void EndProg(void);

Fl_Window *MainWindow = NULL;
Fl_Wizard *Wizard = NULL;
Fl_Button *pNextButton = NULL;

CLangWidget *pLangWidget;

std::list<CBaseWidget *> WidgetList;

void LangMenuCB(Fl_Widget *w, void *) { InstallInfo.cur_lang = ((Fl_Menu_*)w)->mvalue()->text; };
void WizCancelCB(Fl_Widget *, void *) { EndProg(); };
void WizPrevCB(Fl_Widget *, void *) { Wizard->prev(); };
void WizNextCB(Fl_Widget *, void *)
{
    if (Wizard->value()==pLangWidget->GetGroup()) { ReadLang(); UpdateLanguage(); }
    Wizard->next();
    
    for(std::list<CBaseWidget *>::iterator p=WidgetList.begin();p!=WidgetList.end();p++)
    {
        if ((*p)->GetGroup() == Wizard->value())
            (*p)->Activate();
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
    (new Fl_Button(MAIN_WINDOW_W-200, (MAIN_WINDOW_H-30), 80, 25, "@<-    Back"))->callback(WizPrevCB, 0);
    pNextButton = new Fl_Button(MAIN_WINDOW_W-100, (MAIN_WINDOW_H-30), 80, 25, "Next    @->");
    pNextButton->callback(WizNextCB, 0);
    
    Wizard = new Fl_Wizard(20, 20, (MAIN_WINDOW_W-30), (MAIN_WINDOW_H-60), NULL);
    
    CBaseWidget *widget = pLangWidget = new CLangWidget;
    Fl_Group *group = widget->Create();
    if (group) { Wizard->add(group); WidgetList.push_back(widget); }
    
    widget = new CWelcomeWidget;
    group = widget->Create();
    if (group) { Wizard->add(group); WidgetList.push_back(widget); }

    widget = new CLicenseWidget;
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
