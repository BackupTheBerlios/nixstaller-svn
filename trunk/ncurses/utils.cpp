#include "ncurs.h"

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
    char title[] = "Enter name of new directory";
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
        char *error[10];
        char temp[512];
        
        error[0] = "<C>Could not create the directory";
        sprintf (temp, "<C>%.256s", newdir);
        error[1] = copyChar (temp);

        sprintf (temp, "<C>%.256s", strerror (errno));
        error[2] = copyChar (temp);

        /* Pop up the error message. */
        popupLabel(CDKScreen, error, 3);

        /* Clean up and set the error status. */
        freeChar (error[1]);
        freeChar (error[2]);
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
    {
        printf ("Oops. Can't seem to create viewer. Is the window too small?\n");
        return false;
    }

    int lines = CDKreadFile(file, &info);
    if (lines == -1)
    {    
        destroyCDKViewer(Viewer);
        return NO_FILE;
    }

    // Set bottom label
    char *botlabel[3] = { "</B/27>TAB<!27!B>: Go to next button\t\t</B/27>UP/DOWN<!27!B>: Scroll one line up or down",
                          "</B/27>ENTER<!27!B>: Activate current button\t</B/27>LEFT/RIGHT<!27!B>: Move column left or right",
                          "</B/27>ESC<!27!B>: Exit program" };
    SetBottomLabel(botlabel, 3);

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
    setCDKLabelBackgroundColor(BottomLabel, "</B/3>");
    drawCDKLabel(BottomLabel, 1);
    refreshCDKScreen(CDKScreen);
}
