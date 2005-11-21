#include "ncurs.h"

void throwerror(const char *error, ...)
{
    static char txt[256];
    const char *translated = GetTranslation(error);
    va_list v;
    
    va_start(v, error);
        vsprintf(txt, translated, v);
    va_end(v);

    if (BottomLabel) delete BottomLabel;

    if (CDKScreen)
    {
        destroyCDKScreen(CDKScreen);
        endCDK();
    }

    fprintf(stderr, "Error: %s\n", txt);
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
        char *error[3];
        error[0] = CreateText("<C>%s", GetTranslation("Could not create the directory"));
        error[1] = CreateText("<C>%.256s", newdir);
        error[2] = CreateText("<C>%.256s", strerror(errno));

        popupLabel(CDKScreen, error, 3);

        freeChar(newdir);
        return false;
    }
    
    // Update dir box
    std::string dir;
    char **item = NULL;
    CDKALPHALIST *FileList = (CDKALPHALIST *)clientData;
    
    dir = InstallInfo.dest_dir;
        
    dir += "/";
    dir += newdir;
    if (chdir(dir.c_str())) return false;
    
    char tmp[1024];
    if (getcwd(tmp, sizeof(tmp))) InstallInfo.dest_dir = tmp;
        
    int count = ReadDir(InstallInfo.dest_dir, &item);
    if (count == NO_FILE) return false;
        
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
    
    std::vector<void *> *pData = ((std::vector<void *> *)clientData);
    CCDKSWindow *pDescWin = ((CCDKSWindow *)pData->at(0));
    CCDKSWindow *pDefWin = ((CCDKSWindow *)pData->at(1));
    CCDKScroll *pScroll = ((CCDKScroll *)pData->at(2));
    CCharListHelper *items = ((CCharListHelper *)pData->at(3));

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

int ViewFile(char *file, char **buttons, int buttoncount, char *title)
{
    char *button[2], **info = NULL;
    
    CDKVIEWER *Viewer = newCDKViewer(CDKScreen, CENTER, 2, DEFAULT_HEIGHT, DEFAULT_WIDTH, buttons, buttoncount,
                                     A_REVERSE, true, false);

    if (Viewer == NULL)
        throwerror("Can't create text viewer");

    int lines = CDKreadFile(file, &info);
    if (lines == -1)
    {    
        destroyCDKViewer(Viewer);
        return NO_FILE;
    }

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

    /* Set up the viewer title, and the contents to the widget. */
    setCDKViewer(Viewer, title, info, lines, A_REVERSE, true, true, true);
    setCDKViewerBackgroundColor(Viewer, "</B/5");
    
    /* Activate the viewer widget. */
    int selected = activateCDKViewer (Viewer, (chtype *)NULL);

    int ret = (Viewer->exitType == vNORMAL) ? selected : ESCAPE;
    
    /* Clean up. */
    setCDKViewerBackgroundColor(Viewer, "!5!B");
    destroyCDKViewer(Viewer);
    return ret;
}

void SetBottomLabel(char **msg, int count)
{
    if (BottomLabel)
    {
        delete BottomLabel;
        BottomLabel = NULL;
    }

    BottomLabel = new CCDKLabel(CDKScreen, CENTER, BOTTOM, msg, count, true, false);
    if (!BottomLabel)
        throwerror("Can't create bottom text window");
    BottomLabel->SetBgColor(3);
    BottomLabel->Draw();
    refreshCDKScreen(CDKScreen);
}
