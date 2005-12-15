#include "ncurs.h"

void EndProg()
{
    if (CDKScreen)
    {
        ButtonBar.Destroy();
        destroyCDKScreen(CDKScreen);
        endCDK();
    }

    MainEnd();
    exit(0);
}

void throwerror(bool dialog, const char *error, ...)
{
    static char txt[1024];
    const char *translated = GetTranslation(error);
    va_list v;
    
    va_start(v, error);
        vsprintf(txt, translated, v);
    va_end(v);

    if (dialog) WarningBox(txt);
    
    if (CDKScreen)
    {
        ButtonBar.Destroy();
        destroyCDKScreen(CDKScreen);
        endCDK();
    }

    if (!dialog) { fprintf(stderr, GetTranslation("Error: %s"), txt); fprintf(stderr, "\n"); }
    MainEnd();
    exit(EXIT_FAILURE);
}

int ReadDir(const std::string &dir, char ***list)
{
    struct dirent *dirStruct;
    struct stat fileStat;
    int count = 0;
    DIR *dp;
    unsigned used = 0;
   
    dp = opendir (dir.c_str());
    if (dp == 0) return NO_FILE;

    while ((dirStruct = readdir (dp)) != 0)
    {
        if ((lstat(dirStruct->d_name, &fileStat) == 0) && (mode2Filetype(fileStat.st_mode) == 'd') &&
            (strcmp(dirStruct->d_name, ".")))
            used = CDKallocStrings(list, dirStruct->d_name, count++, used);
    }

    closedir (dp);
    sortList (*list, count);
    return count;
}

int SwitchButtonK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData, chtype key GCC_UNUSED)
{
    CDKBUTTONBOX *buttonbox = (CDKBUTTONBOX *)clientData;
    injectCDKButtonbox(buttonbox, key);
    return true;
}

int ScrollParamMenuK(EObjectType cdktype, void *object, void *clientData, chtype key)
{

    if ((key != KEY_DOWN) && (key != KEY_UP)) return true;
    if (cdktype != vSCROLL) return true;
    
    void **pData = ((void **)clientData);
    CCDKSWindow *pDescWin = ((CCDKSWindow *)pData[0]);
    CCDKSWindow *pDefWin = ((CCDKSWindow *)pData[1]);
    CCDKScroll *pScroll = ((CCDKScroll *)pData[2]);
    CCharListHelper *items = ((CCharListHelper *)pData[3]);

    int cur = pScroll->GetCurrent();
    
    if ((key == KEY_DOWN) && ((cur+1) < items->Count())) cur++;
    else if ((key == KEY_UP) && (cur > 0)) cur--;

    for (std::list<command_entry_s *>::iterator p=InstallInfo.command_entries.begin();
         p!=InstallInfo.command_entries.end(); p++)
    {
        if ((*p)->parameter_entries.empty()) continue;

        std::map<std::string, param_entry_s *>::iterator p2;
        p2 = (*p)->parameter_entries.find((*items)[cur]);
        
        if (p2 != (*p)->parameter_entries.end())
        {
            pDescWin->Clear();
            pDescWin->AddText(p2->second->description);
            
            pDefWin->Clear();
            const char *strdef = p2->second->defaultval.c_str(), *strval = p2->second->value.c_str();
            if (p2->second->param_type == PTYPE_BOOL)
            {
                if (!strcmp(strdef, "true")) strdef = GetTranslation("Enabled");
                else strdef = GetTranslation("Disabled");
                if (!strcmp(strval, "true")) strval = GetTranslation("Enabled");
                else strval = GetTranslation("Disabled");
            }
            pDefWin->AddText(CreateText("</B/29>%s:<!29!B> %s", GetTranslation("Default"), strdef), false);
            pDefWin->AddText(CreateText("</B/29>%s:<!29!B> %s", GetTranslation("Current"), strval), false);
            break;
        }
    }
    
    return true;
}

int ExitK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData, chtype key)
{
    WarningBox("Key: %c\n", key);
    return true;
}


int ViewFile(char *file, char **buttons, int buttoncount, char *title)
{
    char **info = NULL;
    
    ButtonBar.Clear();
    ButtonBar.AddButton("TAB", "Next button");
    ButtonBar.AddButton("ENTER", "Activate button");
    ButtonBar.AddButton("Arrows", "Scroll text");
    ButtonBar.AddButton("ESC", "Exit program");
    ButtonBar.Draw();

    int lines = CDKreadFile(file, &info);
    if (lines == -1)
        return NO_FILE;

    CDKVIEWER *Viewer = newCDKViewer(CDKScreen, CENTER, 2, GetMaxHeight()-2, DEFAULT_WIDTH, buttons, buttoncount,
                                     A_REVERSE, true, false);

    if (Viewer == NULL)
        throwerror(false, "Can't create text viewer");

    setCDKViewerBackgroundColor(Viewer, "</B/5");
    setCDKViewer(Viewer, title, info, lines, A_REVERSE, true, true, true);
    
    int selected = activateCDKViewer(Viewer, NULL);
    int ret = (Viewer->exitType == vNORMAL) ? selected : ESCAPE;

    CDKfreeStrings(info);
    setCDKViewerBackgroundColor(Viewer, "!5!B");
    destroyCDKViewer(Viewer);
    refreshCDKScreen(CDKScreen);
    return ret;
}

void WarningBox(const char *msg, ...)
{
    CCharListHelper message;
    static char *buttons[1] = { GetTranslation("OK") };
    static char txt[1024];
    const char *translated = GetTranslation(msg);
    va_list v;
    
    va_start(v, msg);
        vsprintf(txt, translated, v);
    va_end(v);

    // Unwrap text
    std::string unwrapped = txt;
    std::string::size_type index = unwrapped.find("\n"), prevind = 0;
    while (index != std::string::npos)
    {
        message.AddItem(unwrapped.substr(prevind, (index-prevind)));
        prevind = index+1;
        index = unwrapped.find("\n", prevind);
    }
    message.AddItem(unwrapped.substr(prevind, index));
    
    CCDKDialog Diag(CDKScreen, CENTER, CENTER, message, message.Count(), buttons, 1);
    Diag.SetBgColor(26);
    Diag.Activate();
    Diag.Destroy();
    refreshCDKScreen(CDKScreen);
}

bool YesNoBox(const char *msg, ...)
{
    CCharListHelper message;
    static char *buttons[] = { GetTranslation("Yes"), GetTranslation("No") };
    static char txt[1024];
    const char *translated = GetTranslation(msg);
    va_list v;
    
    va_start(v, msg);
    vsprintf(txt, translated, v);
    va_end(v);

    // Unwrap text
    std::string unwrapped = txt;
    std::string::size_type index = unwrapped.find("\n"), prevind = 0;
    while (index != std::string::npos)
    {
        message.AddItem(unwrapped.substr(prevind, (index-prevind)));
        prevind = index+1;
        index = unwrapped.find("\n", prevind);
    }
    message.AddItem(unwrapped.substr(prevind, index));
    
    CCDKDialog Diag(CDKScreen, CENTER, CENTER, message, message.Count(), buttons, 2);
    Diag.SetBgColor(26);
    bool yes = ((Diag.Activate() == 0) && (Diag.ExitType() == vNORMAL));
    Diag.Destroy();
    refreshCDKScreen(CDKScreen);
    return yes;
}

#if 0
// Disabled for now...need a nice way to extract files with root access

// Called during extracting files by libsu
void ExtrThinkFunc(void *p)
{
    // Key mode is unblocked in InstallFiles()
    chtype input = getch();
    if (input == KEY_ESC)
    {
        if (YesNoBox("%s\n%s", GetTranslation("This will abort the installation"), GetTranslation("Are you sure?")))
            EndProg();
    }
}

void PrintExtrOutput(const char *msg, void *p)
{
    static bool percent = true;
    void **pData = ((void **)p);
    CCDKSWindow *pWin = ((CCDKSWindow *)pData[0]);
    CCDKHistogram *pHisto = ((CCDKHistogram *)pData[1]);

    if (percent)
    {
        pHisto->SetValue(0, 100, atoi(msg)/(1+InstallInfo.command_entries.size()));
        pHisto->Draw();
        percent = false;
    }
    else
    {
        pWin->AddText(msg, false);
        percent = true;
    }
}
#endif
