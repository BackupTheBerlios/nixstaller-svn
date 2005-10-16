#include "ncurs.h"

void throwerror(const char *error, ...)
{
    static char txt[256];
    const char *translated = GetTranslation(error);
    va_list v;
    
    va_start(v, error);
        vsprintf(txt, translated, v);
    va_end(v);

    if (BottomLabel) destroyCDKLabel(BottomLabel);
    if (CDKScreen)
    {
        destroyCDKScreen(CDKScreen);
        endCDK();
    }

    fprintf(stderr, "Error: %s\n", txt);
    FreeStrings();
    exit(EXIT_FAILURE);
}

int ReadDir(const char *dir, char ***list)
{
    struct dirent *dirStruct;
    struct stat fileStat;
    int count = 0;
    DIR *dp;
    unsigned used = 0;
   
    dp = opendir (dir);
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

int SwitchButtonK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED, chtype key GCC_UNUSED)
{
    CDKALPHALIST *alist = (CDKALPHALIST *) object;
    
    CDKBUTTONBOX *buttonbox = (CDKBUTTONBOX *)clientData;
    injectCDKButtonbox(buttonbox, key);

    return true;
}

int CreateDirK(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED, void *clientData, chtype key)
{
    if (key != 'c') return true; 
    
    mode_t dirMode = (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH);
    char *title = GetTranslation("Enter name of new directory");
    char label[] = "Dir: ";

    /* Create the entry field widget. */
    CDKENTRY *entry = newCDKEntry(CDKScreen, CENTER, CENTER, title, label, A_NORMAL, '.', vMIXED, 40, 0, 256,
                                  true, false);
    // Set illegal chars
    bindCDKObject (vENTRY, entry, '?', dummyK, 0);
            
    // Draw input box
    setCDKEntryBackgroundColor(entry, "</B/26");
    char *newdir = copyChar(activateCDKEntry(entry, 0));
    setCDKEntryBackgroundColor(entry, "<!26!B");
            
    bool success = ((entry->exitType == vNORMAL) && newdir && newdir[0]);
    
    // Restore screen
    destroyCDKEntry(entry);
    refreshCDKScreen(CDKScreen);
            
    if (!success) return false;

    /* Create the directory. */
    if (mkdir(newdir, dirMode) != 0)
    {
        /* Create the error message. */
        char *error[3];
        
        error[0] = CreateText("<C>%s", GetTranslation("Could not create the directory"));
        error[1] = CreateText("<C>%.256s", newdir);
        error[2] = CreateText("<C>%.256s", strerror(errno));

        /* Pop up the error message. */
        popupLabel(CDKScreen, error, 3);

        /* Clean up and set the error status. */
        freeChar(newdir);
        return false;
    }
    
    // Update dir box
    char tmp[2048];
    char **item = NULL;
    CDKALPHALIST *FileList = (CDKALPHALIST *)clientData;
    
    strcpy(tmp, InstallInfo.dest_dir);
        
    strcat(InstallInfo.dest_dir, "/");
    strcat(InstallInfo.dest_dir, newdir);
    if (chdir(InstallInfo.dest_dir)) { strcpy(InstallInfo.dest_dir, tmp); return false; }
    getcwd(InstallInfo.dest_dir, sizeof(InstallInfo.dest_dir));
        
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
        setCDKLabelBackgroundColor(BottomLabel, "<!3!B>");
        destroyCDKLabel(BottomLabel);
    }
    BottomLabel = newCDKLabel(CDKScreen, CENTER, BOTTOM, msg, count, true, false);
    if (!BottomLabel)
        throwerror("Can't create bottom text window");
    setCDKLabelBackgroundColor(BottomLabel, "</B/3>");
    drawCDKLabel(BottomLabel, 1);
    refreshCDKScreen(CDKScreen);
}
