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
CButtonBar ButtonBar;

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
    if (!(MainWin = initscr())) throwerror(false, "Couldn't init ncurses");
    if (!(CDKScreen = initCDKScreen(MainWin))) throwerror(false, "Couldn't init CDK");
    initCDKColor();
    
    int i=0;
    while(Functions[i])
    {
        if (Functions[i]()) i++;
        else
        {
            char *buttons[2] = { GetTranslation("Yes"), GetTranslation("No") };
            CCharListHelper msg;
    
            msg.AddItem(GetTranslation("This will abort the installation"));
            msg.AddItem(GetTranslation("Are you sure?"));
    
            CCDKDialog dialog(CDKScreen, CENTER, CENTER, msg, msg.Count(), buttons, 2);
            dialog.SetBgColor(26);
            int ret = dialog.Activate();
            if ((ret == 0) && (dialog.ExitType() == vNORMAL)) break;
            dialog.Destroy();
            refreshCDKScreen(CDKScreen);
        }
    }

    EndProg();
    return 0;
}

bool SelectLanguage()
{
    if (InstallInfo.languages.size() == 1)
    {
        InstallInfo.cur_lang = *InstallInfo.languages.begin();
        if (!ReadLang()) throwerror(true, "Couldn't load language file for %s", InstallInfo.cur_lang.c_str());
        return true;
    }
    
    char title[] = "<C></B/29>Please select a language<!29!B>";
    CCharListHelper LangItems;
    
    ButtonBar.Clear();
    ButtonBar.AddButton("Arrows", "Navigate menu");
    ButtonBar.AddButton("ESC", "Exit program");
    ButtonBar.Draw();
    
    for (std::list<char*>::iterator p=InstallInfo.languages.begin();p!=InstallInfo.languages.end();p++)
        LangItems.AddItem(*p);

    CCDKScroll ScrollList(CDKScreen, CENTER, 2, GetMaxHeight()-2, DEFAULT_WIDTH, RIGHT, title, LangItems,
                          LangItems.Count());
    ScrollList.SetBgColor(5);

    CFileDialog FDialog("/home/rick");
    if (FDialog.Activate()) WarningBox("You choose %s", FDialog.Result());

    int selection = ScrollList.Activate();
    
    if (ScrollList.ExitType() == vNORMAL)
    {
        std::list<char *>::iterator it = InstallInfo.languages.begin();
        advance(it, selection);
        InstallInfo.cur_lang = *it;
        if (!ReadLang()) throwerror(true, "Couldn't load language file for %s", InstallInfo.cur_lang.c_str());
    }
    else return false;
    
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
    
    ButtonBar.Clear();
    ButtonBar.AddButton("TAB", "Next button");
    ButtonBar.AddButton("ENTER", "Activate button");
    ButtonBar.AddButton("Arrows", "Navigate menu");
    ButtonBar.AddButton("C", "Create directory");
    ButtonBar.AddButton("ESC", "Exit program");
    ButtonBar.Draw();
    
    if (chdir(InstallInfo.dest_dir.c_str()) != 0)
        throwerror(true, "Couldn't open directory '%s'", InstallInfo.dest_dir.c_str());

    int count = ReadDir(InstallInfo.dest_dir, &item);

    if (count < 1) throwerror(true, "Couldn't read directory %s", InstallInfo.dest_dir.c_str());
    
    CCDKButtonBox ButtonBox(CDKScreen, CENTER, GetMaxHeight()-3, 1,
                            49, 0, 1, 3, buttons, 3);
    ButtonBox.SetBgColor(5);

    CCDKAlphaList FileList(CDKScreen, CENTER, 2, getbegy(ButtonBox.GetBBox()->win)-1, DEFAULT_WIDTH, title, label, item,
                           count);
    FileList.SetBgColor(5);
    setCDKEntryPreProcess(FileList.GetAList()->entryField, CreateDirK, FileList.GetAList());
    FileList.GetAList()->entryField->dispType = vVIEWONLY;  // HACK: Disable backspace
    
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
            WarningBox("%s\n%s", GetTranslation("Could not change to directory"), strerror(errno));
            continue;
        }
        
        char str[1024];
        if (getcwd(str, sizeof(str))) InstallInfo.dest_dir = str;
        else { WarningBox("Could not read current directory"); continue; }
        
        if (item) CDKfreeStrings(item);
        item = NULL;
        
        count = ReadDir(InstallInfo.dest_dir, &item);
        if (count == NO_FILE)
        {
            WarningBox("Could not read directory");
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
    
    ButtonBar.Clear();
    ButtonBar.AddButton("TAB", "Next button");
    ButtonBar.AddButton("ENTER", "Activate button");
    ButtonBar.AddButton("Arrows", "Navigate menu");
    ButtonBar.AddButton("ESC", "Exit program");
    ButtonBar.Draw();
    
    CCDKButtonBox ButtonBox(CDKScreen, CENTER, GetMaxHeight()-3, 1, 68, 0, 1, 3, buttons, 3);
    ButtonBox.SetBgColor(5);

    CCDKScroll ScrollList(CDKScreen, getbegx(ButtonBox.GetBBox()->win), 2, getbegy(ButtonBox.GetBBox()->win)-1, 35, RIGHT,
                          title, ParamItems, ParamItems.Count());
    ScrollList.SetBgColor(5);

    const int defh = 3;
    
    CCDKSWindow DescWindow(CDKScreen, (getbegx(ScrollList.GetScroll()->win) + getmaxx(ScrollList.GetScroll()->win))-1, 2,
                           getmaxy(ScrollList.GetScroll()->win)-defh-1, 34, CreateText("<C></B/29>%s<!29!B>",
                           GetTranslation("Description")), 4);
    DescWindow.SetBgColor(5);
    DescWindow.AddText(pFirstParam->description);
    
    CCDKSWindow DefWindow(CDKScreen, (getbegx(ScrollList.GetScroll()->win) + getmaxx(ScrollList.GetScroll()->win))-1,
                          getmaxy(DescWindow.GetSWin()->win), defh, 34, NULL, 4);
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
    char *title = CreateText("<C></B/29>%s<!29!B>", GetTranslation("Configuring parameters"));
    
    ButtonBar.Clear();
    ButtonBar.AddButton("ESC", "Exit program");
    ButtonBar.Draw();
    
    CCDKSWindow InstallOutput(CDKScreen, 0, 6, GetMaxHeight()-7, -1,
                              CreateText("<C></29/B>%s", GetTranslation("Install output")), 2000);
    InstallOutput.SetBgColor(5);
    nodelay(WindowOf(InstallOutput.GetSWin()), true); // Make sure input doesn't block

    const int halfx = getmaxx(InstallOutput.GetSWin()->win)/2;
    const int maxx = getmaxx(InstallOutput.GetSWin()->win);

    CCDKSWindow ProggWindow(CDKScreen, 0, 2, 5, maxx, NULL, 4);
    ProggWindow.SetBgColor(5);
    ProggWindow.AddText("");
    ProggWindow.AddText(CreateText("</B/29>%s<!29!B>", GetTranslation("Status:")));
    ProggWindow.AddText(CreateText("%s (1/%d)", GetTranslation("Extracting Files"), InstallInfo.command_entries.size()+1),
                        true, BOTTOM, 24);
    
    CCDKHistogram ProgressBar(CDKScreen, 25, 3, 1, maxx-29, HORIZONTAL,
                              CreateText("<C></29/B>%s", GetTranslation("Total Progress")), false);
    ProgressBar.SetBgColor(5);
    ProgressBar.SetHistogram(vPERCENT, TOP, 0, 100, 0, COLOR_PAIR (24) | A_REVERSE | ' ', A_BOLD);
    
    setCDKSwindowLLChar(ProggWindow.GetSWin(), ACS_LTEE);
    setCDKSwindowLRChar(ProggWindow.GetSWin(), ACS_RTEE);
    
    InstallOutput.Draw();
    ProggWindow.Draw();
    
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
        bool askpass = false;
        if ((*it)->need_root != NO_ROOT)
        {
            // Command may need root permission, check if it is so
            if ((*it)->need_root == DEPENDED_ROOT)
            {
                param_entry_s *p = GetParamVar((*it)->dep_param);
                if (p && access(p->value.c_str(), W_OK) != 0)
                {
                    (*it)->need_root = NEED_ROOT;
                    if (!askpass) askpass = true;
                }
            }
            else if (!askpass) askpass = true;
        }

        if (askpass)
        {
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
                        WarningBox("%s\n%s", GetTranslation("Incorrect password given for root user"),
                                   GetTranslation("Please re-type"));
                    }
                    else
                    {
                        throwerror(true, "%s\n%s", GetTranslation("Error: Couldn't use su to gain root access"),
                                   GetTranslation("Make sure you can use su(adding your user to the wheel group may help)"));
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
        else if (percent==-1) throwerror(true, "Error during extracting files");

        ProgressBar.SetValue(0, 100, percent/(1+InstallInfo.command_entries.size()));
        ProgressBar.Draw();

        chtype input = getch();
        if (input == KEY_ESC)
        {
            if (YesNoBox("%s\n%s", GetTranslation("This will abort the installation"), GetTranslation("Are you sure?")))
                EndProg();
        }
    }
    
    percent /= (1+InstallInfo.command_entries.size()); // Convert to overall progress

    short step = 2; // Not 1, because extracting files is also a step
    for (std::list<command_entry_s*>::iterator it=InstallInfo.command_entries.begin();
         it!=InstallInfo.command_entries.end(); it++, step++)
    {
        if ((*it)->command.empty()) continue;

        ProggWindow.Clear();
        ProggWindow.AddText("");
        ProggWindow.AddText(CreateText("</B/29>%s<!29!B>", GetTranslation("Status:")));
        ProggWindow.AddText(CreateText("%s (%d/%d)", GetTranslation((*it)->description.c_str()), step,
                            InstallInfo.command_entries.size()+1), true, BOTTOM, 24);

        ProgressBar.Draw();
        
        std::string command = (*it)->command + " " + GetParameters(*it);
        InstallOutput.AddText("");
        InstallOutput.AddText(CreateText("Execute: %s", command.c_str()));
        InstallOutput.AddText("");
        InstallOutput.AddText("");
        
        if (needrootpw && ((*it)->need_root == NEED_ROOT))
        {
            SuHandler.SetCommand(command);
            if (!SuHandler.ExecuteCommand(passwd))
            {
                throwerror(true, "%s\n('%s')", GetTranslation("Error: Could not execute command"), SuHandler.GetErrorMsgC());
                // UNDONE
            }
        }
        else
        {
            command += " 2> /dev/null";
            FILE *pipe = popen(command.c_str(), "r");
            char term[1024];
            if (pipe)
            {
                while (fgets(term, sizeof(term), pipe))
                {
                    InstallOutput.AddText(term);

                    chtype input = getch();
                    if (input == KEY_ESC) /*injectCDKSwindow(InstallOutput.GetSWin(), input);*/
                    {
                        if (YesNoBox("%s\n%s\n%s", GetTranslation("Install commands are still running"),
                                                   GetTranslation("If you abort now this may lead to a broken installation"),
                                                   GetTranslation("Are you sure?")))
                            EndProg();
                    }
                }
                pclose(pipe);
            }
            else
            {
                throwerror(true, "%s\n('%s')", GetTranslation("Error: Could not execute command"), SuHandler.GetErrorMsgC());
                // UNDONE
            }
            
#if 0
            // Need to find a good way to safely suspend a process...this code doesn't always work :(
            
            int pipefd[2];

            pipe(pipefd);
            pid_t pid = fork();
            if (pid == -1) throwerror(true, "Error during command execution: Could not fork process");
            else if (pid) // Parent process
            {
                close(pipefd[1]); // We're not going to write here

                std::string out;
                char c;
                int compid = -1; // PID of the executed command

                while(read(pipefd[0], &c, sizeof(c)) > 0)
                {
                    out += c;
                    if (c == '\n')
                    {
                        if (compid == -1)
                        {
                            compid = atoi(out.c_str());
                            InstallOutput.AddText(CreateText("pid: %d compid: %d", pid, compid), false);
                        }
                        else InstallOutput.AddText(out, false);
                        out.clear();
                    }

                    chtype input = getch();
                    if (input != ERR) /*injectCDKSwindow(InstallOutput.GetSWin(), input);*/
                    {
                        if (kill(compid, SIGTSTP) < 0) // Pause command execution
                            WarningBox("PID Error: %s\n", strerror(errno));
                        
                        char *buttons[2] = { GetTranslation("Yes"), GetTranslation("No") };
                        CCharListHelper msg;
    
                        msg.AddItem(GetTranslation("This will abort the installation"));
                        msg.AddItem(GetTranslation("Are you sure?"));
    
                        CCDKDialog dialog(CDKScreen, CENTER, CENTER, msg, msg.Count(), buttons, 2);
                        dialog.SetBgColor(26);
                        int ret = dialog.Activate();
                        bool cont = ((ret == 1) || (dialog.ExitType() != vNORMAL));

                        dialog.Destroy();
                        refreshCDKScreen(CDKScreen);

                        if (!cont)
                        {
                            kill(pid, SIGTERM);
                            EndProg();
                        }

                        kill(compid, SIGCONT); // Continue command execution
                    }
                }
                close (pipefd[0]);

                int status;
                //waitpid(pid, &status, 0);
            }
            else // Child process
            {
                // Redirect stdout to pipe
                close(STDOUT_FILENO);
                dup (pipefd[1]);
                
                close (pipefd[0]); // No need to read here

                // Make sure no errors are printed and write pid of new command
                command += " 2> /dev/null &  echo $!";
                execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
                system(CreateText("echo %s", strerror(errno)));
                _exit(1);
            }
#endif
        }

        percent += (1.0f/((float)InstallInfo.command_entries.size()+1.0f))*100.0f;
        ProgressBar.SetValue(0, 100, percent);
    }

    ProgressBar.SetValue(0, 100, 100);
    ProgressBar.Draw();
    
    if (passwd)
    {
        // Nullify pass...just incase
        for (short s=0;s<strlen(passwd);s++) passwd[s] = 0;
        free(passwd);
    }

    // Notify user that installation is done
    CCharListHelper message;
    char *b[1] = { GetTranslation("Exit") };
    
    message.AddItem(CreateText(GetTranslation("Installation of %s complete!"), InstallInfo.program_name));
    message.AddItem(GetTranslation("Press enter to exit"));
    
    CCDKDialog FinishDiag(CDKScreen, CENTER, CENTER, message, message.Count(), b, 1);
    FinishDiag.SetBgColor(26);
    FinishDiag.Activate();

    InstallOutput.Activate();
    return true;
}
