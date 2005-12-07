#include "ncurs.h"

void EndProg()
{
    if (BottomLabel) delete BottomLabel;

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
    
    if (BottomLabel) delete BottomLabel;

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

int ReadDir(std::string &dir, char ***list)
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

int CreateDirK(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED, void *clientData, chtype key)
{
    if (key != 'c') return true; 
    
    mode_t dirMode = (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH);

    CCDKEntry entry(CDKScreen, CENTER, CENTER, GetTranslation("Enter name of new directory"), "", 40, 0, 256);
    
    // Set illegal chars
    entry.Bind('?', dummyK, 0);
            
    // Draw input box
    entry.SetBgColor(26);
    char *newdir = copyChar(entry.Activate());
            
    bool success = ((entry.ExitType() == vNORMAL) && newdir && newdir[0]);
    
    // Restore screen
    entry.Destroy();
    refreshCDKScreen(CDKScreen);
            
    if (!success) return false;

    /* Create the directory. */
    if (mkdir(newdir, dirMode) != 0)
    {
        WarningBox("%s\n%.75s\n%.75s", GetTranslation("Could not create the directory"), newdir, strerror(errno)); 
        return false;
    }
    
    // Update dir box
    std::string dir;
    char **item = NULL;
    CDKALPHALIST *FileList = (CDKALPHALIST *)clientData;
    
    dir = InstallInfo.dest_dir;
        
    dir += "/";
    dir += newdir;
    if (chdir(dir.c_str()))
    {
        WarningBox("%s\n%s", GetTranslation("Could not change to directory"), strerror(errno));
        return false;
    }
    
    char tmp[1024];
    if (getcwd(tmp, sizeof(tmp))) InstallInfo.dest_dir = tmp;
    else { WarningBox("Could not read current directory"); return false; }
        
    int count = ReadDir(InstallInfo.dest_dir, &item);
    if (count == NO_FILE) { WarningBox("Could not read directory"); return false; }
        
    setCDKAlphalist(FileList, item, count, '_', true, false);
    drawCDKAlphalist(FileList, true);

    // HACK: Give textbox content
    setCDKEntryValue(FileList->entryField, chtype2Char(FileList->scrollField->item[FileList->scrollField->currentItem]));
    
    CDKfreeStrings(item);
    freeChar(newdir);

    return false;
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
    char *button[2], **info = NULL;
    
    // Set bottom label
    int x1, x2, y1, y2;
    getbegyx(MainWin, y1, x1);
    getmaxyx(MainWin, y2, x2);
    int txtfieldwidth = ((x2-x1)-16)/2, maxtxtlength = txtfieldwidth-2;
    char *botlabel[4] = { CreateText("</B/27>TAB<!27!B>   : %*.*s</B/27>ENTER<!27!B> : %.*s", -txtfieldwidth, maxtxtlength,
                          GetTranslation("Go to next button"), maxtxtlength,
                          GetTranslation("Activate current button")),
                          "</B/27>  ^<!27!B>",
                          CreateText("</B/27>< <#BU> ><!27!B> : %*.*s</B/27>ESC<!27!B>   : %.*s", -txtfieldwidth, maxtxtlength,
                          GetTranslation("Scroll text"), maxtxtlength, GetTranslation("Exit program")),
                          "</B/27>  v<!27!B>" };
    SetBottomLabel(botlabel, 4);

    CDKVIEWER *Viewer = newCDKViewer(CDKScreen, CENTER, 2, GetMaxHeight()-2, DEFAULT_WIDTH, buttons, buttoncount,
                                     A_REVERSE, true, false);

    if (Viewer == NULL)
        throwerror(false, "Can't create text viewer");

    int lines = CDKreadFile(file, &info);
    if (lines == -1)
    {    
        destroyCDKViewer(Viewer);
        return NO_FILE;
    }

    setCDKViewer(Viewer, title, info, lines, A_REVERSE, true, true, true);
    setCDKViewerBackgroundColor(Viewer, "</B/5");
    
    int selected = activateCDKViewer (Viewer, (chtype *)NULL);
    int ret = (Viewer->exitType == vNORMAL) ? selected : ESCAPE;
    
    setCDKViewerBackgroundColor(Viewer, "!5!B");
    destroyCDKViewer(Viewer);
    refreshCDKScreen(CDKScreen);
    return ret;
}

void SetBottomLabel(char **msg, int count)
{return;
    char *txt[1] = { CreateText("</B/27>TAB<!27!B>: %s </B/27>ENTER<!27!B>: %s </B/27>ARROWS<!27!B>: %s",
        GetTranslation("Next button"), GetTranslation("Activate button"), GetTranslation("Navigate menu")) };
    
    if (!BottomLabel)
    {
        //BottomLabel = new CCDKLabel(CDKScreen, CENTER, BOTTOM, msg, count, true, false);
        BottomLabel = new CCDKLabel(CDKScreen, CENTER, BOTTOM, txt, 1, false);
        if (!BottomLabel)
            throwerror(false, "Could not create bottom text window");
        BottomLabel->SetBgColor(3);
    }
    else
        //BottomLabel->SetText(msg, count);
        BottomLabel->SetText(txt, count);
    BottomLabel->Draw();
    refreshCDKScreen(CDKScreen);
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
