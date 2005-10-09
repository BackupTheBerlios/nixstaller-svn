#include "ncurs.h"

bool SelectLanguage(void);
bool ShowWelcome(void);
bool ShowLicense(void);
bool SelectDir(void);
bool InstallFiles(void);
bool FinishInstall(void);

WINDOW *MainWin;
CDKSCREEN *CDKScreen;
CDKLABEL *BottomLabel = NULL;

bool (*Functions[])(void)  =
{
    *SelectLanguage,
    *ShowWelcome,
    *ShowLicense,
    *SelectDir,
    *InstallFiles,
    NULL
};

int main(int argc, char *argv[])
{
    if (!MainInit(argc, argv))
    {
        printf("Init failed, aborting\n");
        return 1;
    }
    
    // Init
    MainWin = initscr();
    CDKScreen = initCDKScreen(MainWin);
    initCDKColor();
    
    int i=0;
    while(Functions[i])
    {
        if (Functions[i]()) i++;
        else break;
    }
    
    // Deinit
    if (BottomLabel) destroyCDKLabel(BottomLabel);
    if (CDKScreen) destroyCDKScreen(CDKScreen);
    
    endCDK();
    FreeStrings();
    return 0;
}

bool SelectLanguage()
{
    char title[] = "<C></B/29>Please select a language<!29!B>";
    char **items = NULL;
    int count=0;
    unsigned short used = 0;
    
    for (std::list<std::string>::iterator p=InstallInfo.languages.begin();p!=InstallInfo.languages.end();p++)
        used = CDKallocStrings(&items, const_cast<char*>(p->c_str()), count++, used);
        
    sortList(items, count);
    
    CDKSCROLL *ScrollList = newCDKScroll(CDKScreen, CENTER, 2, RIGHT, DEFAULT_HEIGHT, DEFAULT_WIDTH, title, items,
                                         count, false, A_REVERSE, true, false);
    setCDKScrollBackgroundColor(ScrollList, "</B/5>");
    int selection = activateCDKScroll(ScrollList, 0);
    
    bool success = false;
    
    if (ScrollList->exitType == vNORMAL)
    {
        InstallInfo.cur_lang = items[selection];
        success = ReadLang();
    }
        
    CDKfreeStrings(items);
    setCDKScrollBackgroundColor(ScrollList, "<!5!B>");
    destroyCDKScroll(ScrollList);
    return success;
}

bool ShowWelcome()
{
    char *title = CreateText("<C></B/29>%s<!29!B>", GetTranslation("Welcome"));
    char filename[] = "config/welcome";
    char *buttons[2] = { GetTranslation("OK"), GetTranslation("Cancel") };
    return (ViewFile(filename, buttons, 2, title)==0);
}

bool ShowLicense()
{
    char *title = CreateText("<C></B/29>%s<!29!B>", GetTranslation("License Agreement"));
    char filename[] = "config/license";
    char *buttons[2] = { GetTranslation("Agree"), GetTranslation("Decline") };
    
    int ret = ViewFile(filename, buttons, 2, title);
    return (ret==NO_FILE || ret==0);
}

bool SelectDir()
{
    char *buttons[] = { GetTranslation("Open directory"), GetTranslation("Select directory"), GetTranslation("Exit") };
    char *title = CreateText("<C>%s", GetTranslation("Select destination directory"));
    char label[] = "Dir: ";
    char **item = NULL;
    
    int x1, y1, x2, y2;
    getbegyx(MainWin, y1, x1);
    getmaxyx(MainWin, y2, x2);
    
    // Set bottom label
    const int txtfieldwidth = 28, maxtxtlength = 26;
    char *botlabel[3] = { CreateText("</B/27>TAB<!27!B>\t: %*.*s</B/27>UP/DOWN<!27!B>\t: %s", -txtfieldwidth, maxtxtlength,
                                     GetTranslation("Go to next button"), GetTranslation("Highlight previous/next dir")),
                          CreateText("</B/27>ENTER<!27!B>\t: %*.*s</B/27>ESC<!27!B>\t: %s", -txtfieldwidth, maxtxtlength,
                                     GetTranslation("Activate current button"), GetTranslation("Exit program")),
                          CreateText("</B/27>C<!27!B>\t: %.*s", maxtxtlength, GetTranslation("Create new direcotry")) };
    
    SetBottomLabel(botlabel, 3);

    if (chdir(InstallInfo.dest_dir) != 0)
    {
        /* Exit CDK. */
        destroyCDKScreen (CDKScreen);
        endCDK();

        /* Print out a message and exit. */
        printf ("Error: Couldn't change to directory %s\n", InstallInfo.dest_dir);
        exit (EXIT_FAILURE);
    }

    int count = ReadDir(InstallInfo.dest_dir, &item);

    /* Create the scrolling list. */
    CDKALPHALIST *FileList = newCDKAlphalist(CDKScreen, CENTER, 2, DEFAULT_HEIGHT-2, DEFAULT_WIDTH, title, label, item,
                                             count, '_', A_BLINK, true, false);

    /* Is the scrolling list null? */
    if (FileList == 0)
    {
        /* Exit CDK. */
        destroyCDKScreen (CDKScreen);
        endCDK();

        /* Print out a message and exit. */
        printf ("Oops. Could not make scrolling list. Is the window too small?\n");
        exit (EXIT_FAILURE);
    }

    setCDKAlphalistBackgroundColor(FileList, "</B/5>");
    
    CDKBUTTONBOX *ButtonWidget = newCDKButtonbox(CDKScreen, CENTER, getbegy(FileList->win)+FileList->boxHeight-1, 1,
                                                 49, 0, 1, 3, buttons, 3, A_REVERSE, true, false);
    setCDKEntryLLChar(FileList, ACS_LTEE);
    setCDKEntryLRChar(FileList, ACS_RTEE);
    setCDKButtonboxULChar(ButtonWidget, ACS_LTEE);
    setCDKButtonboxURChar(ButtonWidget, ACS_RTEE);
    setCDKButtonboxBackgroundColor(ButtonWidget, "</B/5>");
    drawCDKButtonbox(ButtonWidget, true);
    
    bindCDKObject(vALPHALIST, FileList, KEY_TAB, SwitchButtonK, ButtonWidget);
    setCDKEntryPreProcess(FileList->entryField, CreateDirK, FileList);
    FileList->entryField->dispType = vVIEWONLY;  // HACK: Disable backspace
    
    /* Activate the scrolling list. */
    while(true)
    {
        // HACK: Give textbox content
        setCDKEntryValue(FileList->entryField, chtype2Char(FileList->scrollField->item[FileList->scrollField->currentItem]));

        char *selection = activateCDKAlphalist(FileList, 0);
        if ((FileList->exitType != vNORMAL) || (ButtonWidget->currentButton == 2)) break;
        if (ButtonWidget->currentButton == 1)
        {
            char *dtext[3];
            char *dbuttons[2] = { GetTranslation("OK"), GetTranslation("Cancel") };
            char temp[512];
            
            dtext[0] = CreateText(GetTranslation("This will install %s to the following directory:"), InstallInfo.program_name);
            dtext[1] = InstallInfo.dest_dir;
            dtext[2] = GetTranslation("Continue?");
            CDKDIALOG *Diag = newCDKDialog(CDKScreen, CENTER, CENTER, dtext, 3, dbuttons, 2, COLOR_PAIR(2)|A_REVERSE,
                                           true, true, false);
            setCDKDialogBackgroundColor(Diag, "</B/26>");
    
            /* Activate the dialog box. */
            int selection = activateCDKDialog(Diag, 0);

            freeChar(dtext[0]);
            setCDKDialogBackgroundColor(Diag, "<!26!B>");
            destroyCDKDialog(Diag);
            refreshCDKScreen(CDKScreen);
            if (selection==0) break;
            else continue;
        }
        if (!selection || !selection[0]) continue;

        char tmp[2048];
        strcpy(tmp, InstallInfo.dest_dir);
        
        strcat(InstallInfo.dest_dir, "/");
        strcat(InstallInfo.dest_dir, selection);
        if (chdir(InstallInfo.dest_dir)) { strcpy(InstallInfo.dest_dir, tmp); continue; }
        getcwd(InstallInfo.dest_dir, sizeof(InstallInfo.dest_dir));
        
        if (item) CDKfreeStrings(item);
        item = NULL;
        
        count = ReadDir(InstallInfo.dest_dir, &item);
        if (count == NO_FILE) continue;
        
        setCDKAlphalist(FileList, item, count, '_', true, false);
        drawCDKAlphalist(FileList, true);
    }
    
    bool success = ((FileList->exitType != vESCAPE_HIT) && (ButtonWidget->currentButton == 1));
    
    /* Clean up. */
    setCDKAlphalistBackgroundColor(FileList, "<!5!B>");
    setCDKButtonboxBackgroundColor(ButtonWidget, "<!5!B>");
    CDKfreeStrings(item);
//    destroyCDKLabel(CurrentDirLabel);
    destroyCDKButtonbox (ButtonWidget);
    destroyCDKAlphalist(FileList);

    return success;
}

bool InstallFiles()
{
    char *botlabel[1] = { GetTranslation("Installing files....please wait") };
    SetBottomLabel(botlabel, 1);

    /* Create the histogram. */
    CDKHISTOGRAM *ProgressBar = newCDKHistogram(CDKScreen, CENTER, 2, 1, 0, HORIZONTAL,
                                                CreateText("<C></29/B>%s", GetTranslation("Install Progress")), true, false);

    /* Set the top left/right characters of the histogram.*/
    setCDKHistogramLLChar(ProgressBar, ACS_LTEE);
    setCDKHistogramLRChar(ProgressBar, ACS_RTEE);

    setCDKHistogramBackgroundColor(ProgressBar, "</B/5>");
    
    /* Set the initial value of the histogram. */
    setCDKHistogram (ProgressBar, vPERCENT, TOP, A_BOLD, 1, 100, 0, COLOR_PAIR (24) | A_REVERSE | ' ', true);

    /* Create the scrolling window. */
    CDKSWINDOW *InstallOutput = newCDKSwindow(CDKScreen, CENTER, 5, 12, 0,
                                              CreateText("<C></29/B>%s", GetTranslation("Status")), 2000, true, false);

    /* Set the top left/right characters of the scrolling window.*/
    setCDKSwindowULChar(InstallOutput, ACS_LTEE);
    setCDKSwindowURChar(InstallOutput, ACS_RTEE);

    setCDKSwindowBackgroundColor(InstallOutput, "</B/5>");
    
    drawCDKSwindow(InstallOutput, true);
    drawCDKHistogram(ProgressBar, true);
    refreshCDKScreen(CDKScreen);
    
    short percent = 0;
    while(percent<100)
    {
        char curfile[256], text[300];
        percent = ExtractArchive(curfile);
        
        sprintf(text, "Extracting file: %s", curfile);
        addCDKSwindow(InstallOutput, text, BOTTOM);
        if (percent==100) { strcpy(text, "Done!"); addCDKSwindow(InstallOutput, text, BOTTOM); }
        else if (percent==-1) { return false; }
        drawCDKSwindow(InstallOutput, true);
        
        setCDKHistogram (ProgressBar, vPERCENT, TOP, A_BOLD, 1, 100, percent, COLOR_PAIR (24) | A_REVERSE | ' ', true);
        drawCDKHistogram(ProgressBar, true);
    }
    
    // Notify user that installation is done
    char *message[2];
    char *buttons[1] = { GetTranslation("Exit") };
    char temp[256];
    message[0] = CreateText(GetTranslation("Installation of %s complete"), InstallInfo.program_name);
    message[1] = GetTranslation("Press enter to exit");
    CDKDIALOG *FinishDiag = newCDKDialog(CDKScreen, CENTER, CENTER, message, 2, buttons, 1, COLOR_PAIR(2)|A_REVERSE, true,
                                         true, false);

    /* Check if we got a null value back. */
    if (FinishDiag == 0)
    {
        /* Shut down Cdk. */
        destroyCDKScreen(CDKScreen);
        endCDK();

        /* Spit out a message. */
        printf ("Oops. Can't seem to create the dialog box. Is the window too small?\n");
        exit (EXIT_FAILURE);
    }

    setCDKDialogBackgroundColor(FinishDiag, "</B/26>");
    
    /* Activate the dialog box. */
    activateCDKDialog(FinishDiag, 0);

    freeChar(message[0]);
    setCDKDialogBackgroundColor(FinishDiag, "<!26!B>");
    setCDKHistogramBackgroundColor(ProgressBar, "<!5!B>");
    setCDKSwindowBackgroundColor(InstallOutput, "<!5!B>");
    destroyCDKDialog(FinishDiag);
    destroyCDKHistogram(ProgressBar);
    destroyCDKSwindow(InstallOutput);

    return true;
}
