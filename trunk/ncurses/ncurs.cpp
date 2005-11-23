#include "ncurs.h"

bool SelectLanguage(void);
bool ShowWelcome(void);
bool ShowLicense(void);
bool SelectDir(void);
bool ConfParams(void);
bool InstallFiles(void);
bool FinishInstall(void);

WINDOW *MainWin = NULL;
CDKSCREEN *CDKScreen = NULL;
CCDKLabel *BottomLabel = NULL;

bool (*Functions[])(void)  =
{
    *SelectLanguage,
    *ShowWelcome,
    *ShowLicense,
    *SelectDir,
    *ConfParams,
    *InstallFiles,
    NULL
};

int main(int argc, char *argv[])
{
    if (!MainInit(argc, argv))
    {
        printf("Error: %s\n", GetTranslation("Init failed, aborting"));
        return 1;
    }
    
    // Init
    if (!(MainWin = initscr())) throwerror(false, GetTranslation("Couldn't init ncurses"));
    if (!(CDKScreen = initCDKScreen(MainWin))) throwerror(false, GetTranslation("Couldn't init CDK"));
    initCDKColor();
    
    int i=0;
    while(Functions[i] && Functions[i]()) i++;
    
    // Deinit
    if (BottomLabel) delete BottomLabel;
    if (CDKScreen) destroyCDKScreen(CDKScreen);
    
    endCDK();
    MainEnd();
    return 0;
}

bool SelectLanguage()
{
    if (InstallInfo.languages.size() == 1)
    {
        InstallInfo.cur_lang = *InstallInfo.languages.begin();
        if (!ReadLang()) throwerror(true, GetTranslation("Couldn't load language file for %s"), InstallInfo.cur_lang.c_str());
        return true;
    }
    
    char title[] = "<C></B/29>Please select a language<!29!B>";
    CCharListHelper LangItems;
    CCharListHelper botlabel;
    int x1, x2, y1, y2;
    getbegyx(MainWin, y1, x1);
    getmaxyx(MainWin, y2, x2);
    int txtfieldwidth = ((x2-x1)-16)/2, maxtxtlength = txtfieldwidth-2;
    
    botlabel.AddItem("</B/27>  ^<!27!B>");
    botlabel.AddItem("</B/27>  <#BU>  <!27!B> : Highlight previous/next language\t\t"
                     "</B/27>ESC<!27!B>   : Exit program");
    botlabel.AddItem("</B/27>  v<!27!B>");
    SetBottomLabel(botlabel, botlabel.Count());
    
    for (std::list<char*>::iterator p=InstallInfo.languages.begin();p!=InstallInfo.languages.end();p++)
        LangItems.AddItem(*p);

    CCDKScroll ScrollList(CDKScreen, CENTER, CENTER, DEFAULT_HEIGHT, DEFAULT_WIDTH, RIGHT, title, LangItems,
                          LangItems.Count());
    ScrollList.SetBgColor(5);
    
    int selection = ScrollList.Activate();
    
    if (ScrollList.ExitType() == vNORMAL)
    {
        std::list<char *>::iterator it = InstallInfo.languages.begin();
        advance(it, selection);
        InstallInfo.cur_lang = *it;
        if (!ReadLang()) throwerror(true, GetTranslation("Couldn't load language file for %s"), InstallInfo.cur_lang.c_str());
    }
        
    return true;
}

bool ShowWelcome()
{
    char *title = CreateText("<C></B/29>%s<!29!B>", GetTranslation("Welcome"));
    char filename[] = "config/welcome";
    char *buttons[2] = { GetTranslation("OK"), GetTranslation("Cancel") };
    int ret = ViewFile(filename, buttons, 2, title);
    return (ret==NO_FILE || ret==0);
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
    if (InstallInfo.dest_dir_type != DEST_SELECT) return true;
    
    char *buttons[] = { GetTranslation("Open directory"), GetTranslation("Select directory"), GetTranslation("Exit") };
    char *title = CreateText("<C>%s", GetTranslation("Select destination directory"));
    char label[] = "Dir: ";
    char **item = NULL;
    
    // Set bottom label
    int x1, x2, y1, y2;
    getbegyx(MainWin, y1, x1);
    getmaxyx(MainWin, y2, x2);
    int txtfieldwidth = ((x2-x1)-16)/2, maxtxtlength = txtfieldwidth-2;
    CCharListHelper botlabel;
    
    botlabel.AddItem(CreateText("</B/27>TAB<!27!B>   : %*.*s</B/27>ENTER<!27!B> : %.*s", -txtfieldwidth, maxtxtlength,
                                GetTranslation("Go to next button"), maxtxtlength, GetTranslation("Activate current button")));
    botlabel.AddItem("</B/27>  ^<!27!B>");
    botlabel.AddItem(CreateText("</B/27>  <#BU>  <!27!B> : %*.*s</B/27>ESC<!27!B>   : %.*s", -txtfieldwidth, maxtxtlength,
                                GetTranslation("Highlight previous/next dir"), maxtxtlength, GetTranslation("Exit program")));
    botlabel.AddItem("</B/27>  v<!27!B>");
    botlabel.AddItem(CreateText("</B/27>C<!27!B>     : %*.*s", -txtfieldwidth, maxtxtlength,
                                GetTranslation("Create new directory")));
    SetBottomLabel(botlabel, botlabel.Count());

    if (chdir(InstallInfo.dest_dir.c_str()) != 0)
        throwerror(true, GetTranslation("Couldn't open directory '%s'"), InstallInfo.dest_dir.c_str());

    int count = ReadDir(InstallInfo.dest_dir, &item);

    if (count < 1) throwerror(true, GetTranslation("Couldn't read directory %s"), InstallInfo.dest_dir.c_str());
    
    CCDKAlphaList FileList(CDKScreen, CENTER, 2, DEFAULT_HEIGHT-2, DEFAULT_WIDTH, title, label, item, count);
    FileList.SetBgColor(5);
    setCDKEntryPreProcess(FileList.GetAList()->entryField, CreateDirK, FileList.GetAList());
    FileList.GetAList()->entryField->dispType = vVIEWONLY;  // HACK: Disable backspace

    CCDKButtonBox ButtonBox(CDKScreen, CENTER, getbegy(FileList.GetAList()->win)+FileList.GetAList()->boxHeight-1, 1,
                            49, 0, 1, 3, buttons, 3);
    ButtonBox.SetBgColor(5);
    
    setCDKAlphalistLLChar(FileList.GetAList(), ACS_LTEE);
    setCDKAlphalistLRChar(FileList.GetAList(), ACS_RTEE);
    setCDKButtonboxULChar(ButtonBox.GetBBox(), ACS_LTEE);
    setCDKButtonboxURChar(ButtonBox.GetBBox(), ACS_RTEE);
    
    FileList.Draw();
    ButtonBox.Draw();
    
    FileList.Bind(KEY_TAB, SwitchButtonK, ButtonBox.GetBBox()); // Pas TAB through ButtonBox

    /* Activate the scrolling list. */
    while(true)
    {
        // HACK: Give textbox content
        setCDKEntryValue(FileList.GetAList()->entryField,
                         chtype2Char(FileList.GetAList()->scrollField->item[FileList.GetAList()->scrollField->currentItem]));

        char *selection = FileList.Activate();
        if ((FileList.ExitType() != vNORMAL) || (ButtonBox.GetCurrent() == 2)) break;
        if (ButtonBox.GetCurrent() == 1)
        {
            CCharListHelper dtext;
            char *dbuttons[2] = { GetTranslation("OK"), GetTranslation("Cancel") };
            
            dtext.AddItem(CreateText(GetTranslation("This will install %s to the following directory:"),
                                     InstallInfo.program_name));
            dtext.AddItem(InstallInfo.dest_dir);
            dtext.AddItem(GetTranslation("Continue?"));
            
            CCDKDialog Diag(CDKScreen, CENTER, CENTER, dtext, dtext.Count(), dbuttons, 2);
            Diag.SetBgColor(26);
    
            int sel = Diag.Activate();

            Diag.Destroy();
            refreshCDKScreen(CDKScreen);
            
            if (sel==0) break;
            else continue;
        }
        if (!selection || !selection[0]) continue;

        std::string dir = InstallInfo.dest_dir;
        
        dir += "/";
        dir += selection;
        if (chdir(dir.c_str()))
        {
            WarningBox(CreateText("%s\n%s", GetTranslation("Could not change to directory"), strerror(errno)));
            continue;
        }
        
        char str[1024];
        if (getcwd(str, sizeof(str))) InstallInfo.dest_dir = str;
        
        if (item) CDKfreeStrings(item);
        item = NULL;
        
        count = ReadDir(InstallInfo.dest_dir, &item);
        if (count == NO_FILE)
        {
            WarningBox(GetTranslation("Could not read directory"));
            continue;
        }
        
        FileList.SetContent(item, count);
        FileList.Draw();
    }
    
    bool success = ((FileList.ExitType() != vESCAPE_HIT) && (ButtonBox.GetCurrent() == 1));
    
    CDKfreeStrings(item);

    return success;
}

bool ConfParams()
{
    param_entry_s *pFirstParam = NULL;
    CCharListHelper ParamItems;

    for (std::list<command_entry_s *>::iterator p=InstallInfo.command_entries.begin();p!=InstallInfo.command_entries.end();
         p++)
    {
        for (std::map<std::string, param_entry_s *>::iterator p2=(*p)->parameter_entries.begin();
             p2!=(*p)->parameter_entries.end();p2++)
        {
            if (!pFirstParam) pFirstParam = p2->second;
            ParamItems.AddItem(p2->first);
        }
    }

    if (!pFirstParam) return true; // No command entries...no need for this screen
    
    char *title = CreateText("<C></B/29>%s<!29!B>", GetTranslation("Configuring parameters"));
    char *buttons[3] = { GetTranslation("Edit parameter"), GetTranslation("Continue install"), GetTranslation("Cancel") };
    CCharListHelper botlabel;
    int x1, x2, y1, y2;
    getbegyx(MainWin, y1, x1);
    getmaxyx(MainWin, y2, x2);
    int txtfieldwidth = ((x2-x1)-16)/2, maxtxtlength = txtfieldwidth-2;
    
    botlabel.AddItem(CreateText("</B/27>TAB<!27!B>   : %*.*s</B/27>ENTER<!27!B> : %.*s", -txtfieldwidth, maxtxtlength,
                     GetTranslation("Go to next button"), maxtxtlength, GetTranslation("Activate current button")));
    botlabel.AddItem("</B/27>  ^<!27!B>");
    botlabel.AddItem(CreateText("</B/27>  <#BU>  <!27!B> : %*.*s</B/27>ESC<!27!B>   : %.*s", -txtfieldwidth, maxtxtlength,
                     GetTranslation("Highlight previous/next parameter"), maxtxtlength, GetTranslation("Exit program")));
    botlabel.AddItem("</B/27>  v<!27!B>");
    SetBottomLabel(botlabel, botlabel.Count());
    
    CCDKButtonBox ButtonBox(CDKScreen, CENTER, DEFAULT_HEIGHT, 1, 68, 0, 1, 3, buttons, 3);
    ButtonBox.SetBgColor(5);

    CCDKScroll ScrollList(CDKScreen, getbegx(ButtonBox.GetBBox()->win), 2, DEFAULT_HEIGHT-1, 35, RIGHT, title, ParamItems,
                          ParamItems.Count());
    ScrollList.SetBgColor(5);
    
    CCDKSWindow DescWindow(CDKScreen, getbegx(ButtonBox.GetBBox()->win)+35, 2, 5, 34, CreateText("<C></B/29>%s<!29!B>",
                           GetTranslation("Description")), 4);
    DescWindow.SetBgColor(5);
    DescWindow.AddText(pFirstParam->description);
    
    CCDKSWindow DefWindow(CDKScreen, getbegx(ButtonBox.GetBBox()->win)+35, 8, 4, 34, /*CreateText("<C></B/29>%s<!29!B>",
                          GetTranslation("Default"))*/NULL, 4);
    DefWindow.SetBgColor(5);
    
    const char *str = pFirstParam->defaultval.c_str();
    if (pFirstParam->param_type == PTYPE_BOOL)
    {
        if (!strcmp(str, "true")) str = GetTranslation("Enabled");
        else str = GetTranslation("Disabled");
    }
    
    DefWindow.AddText(CreateText("</B/29>%s:<!29!B> %s", GetTranslation("Default"), str), false);
    DefWindow.AddText(CreateText("</B/29>%s:<!29!B> %s", GetTranslation("Current"), str), false);

    setCDKScrollLLChar(ScrollList.GetScroll(), ACS_LTEE);
    setCDKScrollLRChar(ScrollList.GetScroll(), ACS_BTEE);
    setCDKScrollURChar(ScrollList.GetScroll(), ACS_TTEE);
    setCDKSwindowULChar(DescWindow.GetSWin(), ACS_TTEE);
    setCDKSwindowLLChar(DescWindow.GetSWin(), ACS_LTEE);
    setCDKSwindowLRChar(DescWindow.GetSWin(), ACS_RTEE);
    setCDKSwindowURChar(DefWindow.GetSWin(), ACS_RTEE);
    setCDKSwindowLRChar(DefWindow.GetSWin(), ACS_RTEE);
    setCDKButtonboxULChar(ButtonBox.GetBBox(), ACS_LTEE);
    setCDKButtonboxURChar(ButtonBox.GetBBox(), ACS_RTEE);
    
    ButtonBox.Draw();
    DescWindow.Draw();
    DefWindow.Draw();
    
    void *Data[4] = { &DescWindow, &DefWindow, &ScrollList, &ParamItems };
    setCDKScrollPreProcess(ScrollList.GetScroll(), ScrollParamMenuK, &Data);

    ScrollList.Bind(KEY_TAB, SwitchButtonK, ButtonBox.GetBBox());
    
    bool success = false;
    while(1)
    {
        int selection = ScrollList.Activate();
    
        if (ScrollList.ExitType() == vNORMAL)
        {
            if (ButtonBox.GetCurrent() == 1)
            {
                success = true;
                break;
            }
            else if (ButtonBox.GetCurrent() == 2)
            {
                success = false;
                break;
            }
            for (std::list<command_entry_s *>::iterator p=InstallInfo.command_entries.begin();
                 p!=InstallInfo.command_entries.end(); p++)
            {
                if ((*p)->parameter_entries.empty()) continue;
                std::map<std::string, param_entry_s *>::iterator p2;
                p2 = (*p)->parameter_entries.find(ParamItems[selection]);
                if (p2 != (*p)->parameter_entries.end())
                {
                    if ((p2->second->param_type == PTYPE_STRING) ||
                        (p2->second->param_type == PTYPE_DIR))
                    {
                        CCDKEntry entry(CDKScreen, CENTER, CENTER, GetTranslation("Please enter new value"), "", 40, 0, 256);
                        entry.SetBgColor(26);
                        entry.SetValue(p2->second->value);
                        
                        const char *newval = entry.Activate();
                        
                        if ((entry.ExitType() == vNORMAL) && newval)
                            p2->second->value = newval;
                        
                        // Restore screen
                        entry.Destroy();
                        refreshCDKScreen(CDKScreen);
                    }
                    else
                    {
                        CCharListHelper chitems;
                        if (p2->second->param_type == PTYPE_BOOL)
                        {
                            chitems.AddItem(GetTranslation("Disable"));
                            chitems.AddItem(GetTranslation("Enable"));
                        }
                        else
                        {
                            for (std::list<std::string>::iterator it=p2->second->options.begin();
                                 it!=p2->second->options.end();it++)
                                chitems.AddItem(*it);
                        }
                        CCDKScroll chScrollList(CDKScreen, CENTER, CENTER, 6, 30, RIGHT,
                                                GetTranslation("Please choose new value"), chitems, chitems.Count());
                        chScrollList.SetBgColor(26);
                        int chsel = chScrollList.Activate();
                        
                        if (chScrollList.ExitType() == vNORMAL)
                        {
                            if (p2->second->param_type == PTYPE_BOOL)
                            {
                                if (!strcmp(chitems[chsel], GetTranslation("Enable"))) p2->second->value = "true";
                                else p2->second->value = "false";
                            }
                            else
                                p2->second->value = chitems[chsel];
                        }
                            
                        chScrollList.Destroy();
                        refreshCDKScreen(CDKScreen);
                    }
                    break;
                }
            }
        }
        else
        {
            success = false;
            break;
        }
    }
    
    return success;
}

bool InstallFiles()
{
    char *botlabel[1] = { GetTranslation("Installing files...please wait") };
    SetBottomLabel(botlabel, 1);

    CCDKHistogram ProgressBar(CDKScreen, CENTER, 2, 1, 0, HORIZONTAL,
                              CreateText("<C></29/B>%s", GetTranslation("Install Progress")));

    setCDKHistogramLLChar(ProgressBar.GetHistogram(), ACS_LTEE);
    setCDKHistogramLRChar(ProgressBar.GetHistogram(), ACS_RTEE);
    ProgressBar.SetBgColor(5);
    
    ProgressBar.SetHistogram(vPERCENT, TOP, 0, 100, 0, COLOR_PAIR (24) | A_REVERSE | ' ', A_BOLD);

    CCDKSWindow InstallOutput(CDKScreen, CENTER, 5, 12, 0, CreateText("<C></29/B>%s", GetTranslation("Status")), 2000);

    setCDKSwindowULChar(InstallOutput.GetSWin(), ACS_LTEE);
    setCDKSwindowURChar(InstallOutput.GetSWin(), ACS_RTEE);
    InstallOutput.SetBgColor(5);
    
    InstallOutput.Draw();
    ProgressBar.Draw();
    refreshCDKScreen(CDKScreen);
    
    // Check if we need root access
    char *passwd = NULL;
    CLibSU SuHandler;
    SuHandler.SetOutputFunc(PrintInstOutput, &InstallOutput);
    SuHandler.SetPath("/bin:/usr/bin:/usr/local/bin");
    SuHandler.SetUser("root");
    SuHandler.SetTerminalOutput(false);
    
    for (std::list<command_entry_s *>::iterator it=InstallInfo.command_entries.begin();
        it!=InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->need_root != NO_ROOT)
        {
            // Command may need root permission, check if it is so
            if ((*it)->need_root == DEPENDED_ROOT)
            {
                param_entry_s *p = GetParamVar((*it)->dep_param);
                if (p && access(p->value.c_str(), W_OK) != 0)
                {
                    (*it)->need_root = NEED_ROOT;
                }
                else continue;
            }
            
            if (passwd) continue; // No need to ask pass more than once...
            
            CCDKEntry entry(CDKScreen, CENTER, CENTER, GetTranslation("Please enter root password"), "", 40, 0, 256, vHMIXED);
            entry.SetHiddenChar('*');
            entry.SetBgColor(26);
            
            while(1)
            {
                char *sz = entry.Activate();
                        
                if ((entry.ExitType() != vNORMAL) || !sz)
                {
                    char *buttons[2] = { GetTranslation("OK"), GetTranslation("Cancel") };
                    CCharListHelper msg;
    
                    msg.AddItem(GetTranslation("Root access is required to continue."));
                    msg.AddItem(GetTranslation("Abort installation?"));
    
                    CCDKDialog dialog(CDKScreen, CENTER, CENTER, msg, msg.Count(), buttons, 2);
                    dialog.SetBgColor(26);
                    int ret = dialog.Activate();
                    if ((ret == 1) || (dialog.ExitType() != vNORMAL))
                    {
                        MainEnd();
                        exit(-1);
                    }
                    refreshCDKScreen(CDKScreen);
                }
                else
                {
                    if (SuHandler.TestSU(sz))
                    {
                        passwd = strdup(sz);
                        for (short s=0;s<strlen(sz);s++) sz[s] = 0;
                        break;
                    }
                    
                    for (short s=0;s<strlen(sz);s++) sz[s] = 0;
                    
                    // Some error appeared
                    if (SuHandler.GetError() == CLibSU::SU_ERROR_INCORRECTPASS)
                    {
                        char *buttons[2] = { GetTranslation("OK") };
                        CCharListHelper msg;
    
                        msg.AddItem(GetTranslation("Incorrect password given for root user"));
                        msg.AddItem(GetTranslation("Please re-type"));
    
                        CCDKDialog dialog(CDKScreen, CENTER, CENTER, msg, msg.Count(), buttons, 1);
                        dialog.SetBgColor(26);
                        int ret = dialog.Activate();
                    }
                    else
                    {
                        throwerror(true, "%s\n%s", GetTranslation("Error: Couldn't use su to gain root access"),
                                   GetTranslation("Make sure you can use su(adding your user to the wheel group may help"));
                    }
                }
            }
            // Restore screen
            entry.Destroy();
            refreshCDKScreen(CDKScreen);
        }
    }
    
    bool needrootpw = SuHandler.NeedPassword();
    short percent = 0;
    while(percent<100)
    {
        char curfile[256], text[300];
        percent = ExtractArchive(curfile);
        
        sprintf(text, "Extracting file: %s", curfile);
        InstallOutput.AddText(text, false);
        if (percent==100) InstallOutput.AddText("Done!", false);
        else if (percent==-1) throwerror(true, "Could not extract files");
        InstallOutput.Draw();
        
        ProgressBar.SetValue(0, 100, percent);
        ProgressBar.Draw();
    }
    
    ProgressBar.SetValue(0, 100, 0);
    ProgressBar.Draw();
    percent = 0;
        
    for (std::list<command_entry_s*>::iterator it=InstallInfo.command_entries.begin();
            it!=InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->command.empty()) continue;
        std::string command = (*it)->command + " " + GetParameters(*it);
        InstallOutput.AddText("", false);
        InstallOutput.AddText(CreateText("Execute: %s", command.c_str()), false);
        InstallOutput.AddText("", false);
        InstallOutput.AddText("", false);
        if (needrootpw && ((*it)->need_root == NEED_ROOT))
        {
            SuHandler.SetCommand(command);
            if (!SuHandler.ExecuteCommand(passwd))
                throwerror(true, GetSUErrorMsg(&SuHandler));
        }
        else
        {
            command += " 2> /dev/null";
            FILE *pipe = popen(command.c_str(), "r");
            char term[256];
            if (pipe)
            {
                while (fgets(term, sizeof(term), pipe)) InstallOutput.AddText(term, false);
                pclose(pipe);
            }
            else throwerror(true, "Couldn't open pipe");
        }
        percent += (1.0f/(float)InstallInfo.command_entries.size())*100.0f;
        ProgressBar.SetValue(0, 100, percent);
        ProgressBar.Draw();
    }
    
    if (passwd)
    {
        // Nullify pass...just incase
        for (short s=0;s<strlen(passwd);s++) passwd[s] = 0;
        free(passwd);
    }

    // Notify user that installation is done
    CCharListHelper message;
    char *buttons[1] = { GetTranslation("Exit") };
    
    message.AddItem(CreateText(GetTranslation("Installation of %s complete!"), InstallInfo.program_name));
    message.AddItem(GetTranslation("Press enter to exit"));
    
    CCDKDialog FinishDiag(CDKScreen, CENTER, CENTER, message, message.Count(), buttons, 1);
    FinishDiag.SetBgColor(26);
    FinishDiag.Activate();

    return true;
}
