/*
    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.

    Nixstaller is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Nixstaller is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nixstaller; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
    conditions of the GNU General Public License cover the whole combination.

    In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
    software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
    DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
    such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
    you include the source code of that other code when and as the GNU GPL requires distribution of source code.

    Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
    versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
    version without this exception; this exception also makes it possible to release a modified version which carries forward
    this exception.
*/

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
    *FinishInstall,
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
    if (!(MainWin = initscr())) throwerror(false, "Could not init ncurses");
    if (!(CDKScreen = initCDKScreen(MainWin))) throwerror(false, "Could not init CDK");
    initCDKColor();

    if ((InstallInfo.dest_dir_type == DEST_DEFAULT) && !WriteAccess(InstallInfo.dest_dir))
        throwerror(true, CreateText("This installer will install files to the following directory:\n%s\n"
                                    "However you don't have write permissions to this directory"
                                    "Please restart the installer as a user who does or as the root user",
                                    InstallInfo.dest_dir.c_str()));
    
    int i=0;
    while(Functions[i])
    {
        if (Functions[i]()) i++;
        else
        {
            char *buttons[2] = { GetTranslation("Yes"), GetTranslation("No") };
            char *msg[2] = { GetTranslation("This will abort the installation"), GetTranslation("Are you sure?") };
            
            CCDKDialog dialog(CDKScreen, CENTER, CENTER, msg, 2, buttons, 2);
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
        InstallInfo.cur_lang = InstallInfo.languages.front();
        if (!ReadLang()) throwerror(true, "Could not load language file for %s", InstallInfo.cur_lang.c_str());
        return true;
    }
    
    char title[] = "<C></B/29>Please select a language<!29!B>";
    std::vector<char *> LangItems;
    
    ButtonBar.Clear();
    ButtonBar.AddButton("Arrows", "Navigate menu");
    ButtonBar.AddButton("A", "About");
    ButtonBar.AddButton("ESC", "Exit program");
    ButtonBar.Draw();
    
    for (std::list<std::string>::iterator p=InstallInfo.languages.begin();p!=InstallInfo.languages.end();p++)
        LangItems.push_back(MakeCString(*p));

    CCDKScroll ScrollList(CDKScreen, CENTER, CENTER, GetMaxHeight(20), GetMaxWidth(45), RIGHT, title, &LangItems[0],
                          LangItems.size());
    ScrollList.SetBgColor(5);
    ScrollList.Bind('a', ShowAboutK);

    int selection = ScrollList.Activate();
    
    if (ScrollList.ExitType() == vNORMAL)
    {
        InstallInfo.cur_lang = LangItems[selection];
        ReadLang();
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
    char *title = CreateText("<C></B/29>%s<!29!B>", GetTranslation("License agreement"));
    char filename[] = "config/license";
    char *buttons[2] = { GetTranslation("Agree"), GetTranslation("Decline") };
    
    int ret = ViewFile(filename, buttons, 2, title);
    return (ret==NO_FILE || ret==0);
}

bool SelectDir()
{
    if (InstallInfo.dest_dir_type != DEST_SELECT) return true;
    CFileDialog FDialog(InstallInfo.dest_dir, CreateText("<C>%s", GetTranslation("Select destination directory")), false,
                        true);

    while (true)
    {
        if (FDialog.Activate())
        {
            InstallInfo.dest_dir = FDialog.Result();
            return true;
        }
        else break;
    }
    return false;
}

bool ConfParams()
{
    param_entry_s *pFirstParam = NULL;
    std::vector<char *> ParamItems;

    for (std::list<command_entry_s *>::iterator p=InstallInfo.command_entries.begin();p!=InstallInfo.command_entries.end();
         p++)
    {
        for (std::map<std::string, param_entry_s *>::iterator p2=(*p)->parameter_entries.begin();
             p2!=(*p)->parameter_entries.end();p2++)
        {
            if (!pFirstParam) pFirstParam = p2->second;
            ParamItems.push_back(MakeCString(p2->first));
        }
    }

    if (!pFirstParam) return true; // No command entries...no need for this screen
    
    char *title = CreateText("<C></B/29>%s<!29!B>", GetTranslation("Configure parameters"));
    char *buttons[3] = { GetTranslation("Edit parameter"), GetTranslation("Continue"), GetTranslation("Cancel") };
    
    ButtonBar.Clear();
    ButtonBar.AddButton("TAB", "Next button");
    ButtonBar.AddButton("ENTER", "Activate button");
    ButtonBar.AddButton("Arrows", "Navigate menu");
    ButtonBar.AddButton("A", "About");
    ButtonBar.AddButton("ESC", "Exit program");
    ButtonBar.Draw();
    
    CCDKButtonBox ButtonBox(CDKScreen, CENTER, GetDefaultHeight()-1, 1, GetDefaultWidth(), 0, 1, 3, buttons, 3);
    ButtonBox.SetBgColor(5);

    CCDKScroll ScrollList(CDKScreen, getbegx(ButtonBox.GetBBox()->win), 2, getbegy(ButtonBox.GetBBox()->win)-1,
                          GetDefaultWidth()/2, RIGHT,
                          title, &ParamItems[0], ParamItems.size());
    ScrollList.SetBgColor(5);
    ScrollList.Bind('a', ShowAboutK);

    const int defh = 3;
    
    CCDKSWindow DescWindow(CDKScreen, (getbegx(ScrollList.GetScroll()->win) + getmaxx(ScrollList.GetScroll()->win))-1, 2,
                           getmaxy(ScrollList.GetScroll()->win)-defh-1, (GetDefaultWidth()/2)+1, CreateText("<C></B/29>%s<!29!B>",
                           GetTranslation("Description")), 30);
    DescWindow.SetBgColor(5);
    DescWindow.AddText(GetTranslation(pFirstParam->description));
    
    CCDKSWindow DefWindow(CDKScreen, (getbegx(ScrollList.GetScroll()->win) + getmaxx(ScrollList.GetScroll()->win))-1,
                          getmaxy(DescWindow.GetSWin()->win), defh, (GetDefaultWidth()/2)+1, NULL, 4);
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
            
            param_entry_s *pParam = GetParamByName(ParamItems[selection]);
            
            if (pParam->param_type == PTYPE_DIR)
            {
                CFileDialog filedialog(pParam->value, CreateText("<C>%s", GetTranslation("Select a directory")),
                                       true, false);
                if (filedialog.Activate()) pParam->value = filedialog.Result();
                filedialog.Destroy();
                refreshCDKScreen(CDKScreen);
            }
            else if (pParam->param_type == PTYPE_STRING)
            {
                CCDKEntry entry(CDKScreen, CENTER, CENTER, GetTranslation("Please enter new value"), "", 40, 0, 256);
                entry.SetBgColor(26);
                entry.SetValue(pParam->value);
                
                const char *newval = entry.Activate();
                
                if ((entry.ExitType() == vNORMAL) && newval)
                    pParam->value = newval;
                
                // Restore screen
                entry.Destroy();
                refreshCDKScreen(CDKScreen);
            }
            else
            {
                std::vector<char *> chitems;
                int cur = 0;
                
                if (pParam->param_type == PTYPE_BOOL)
                {
                    chitems.push_back(GetTranslation("Disable"));
                    chitems.push_back(GetTranslation("Enable"));
                    cur = (pParam->value == "true") ? 1 : 0;
                }
                else
                {
                    int i = 0;
                    for (std::list<std::string>::iterator it=pParam->options.begin(); it!=pParam->options.end();it++, i++)
                    {
                        chitems.push_back(GetTranslation(MakeCString(*it)));
                        if (*it == pParam->value)
                            cur = i;
                    }
                }
                
                CCDKScroll chScrollList(CDKScreen, CENTER, CENTER, 6, 30, RIGHT,
                                        GetTranslation("Please choose new value"), &chitems[0], chitems.size());
                chScrollList.SetBgColor(26);
                chScrollList.SetCurrent(cur);
                
                int chsel = chScrollList.Activate();
                
                if (chScrollList.ExitType() == vNORMAL)
                {
                    if (pParam->param_type == PTYPE_BOOL)
                    {
                        if (!strcmp(chitems[chsel], GetTranslation("Enable"))) pParam->value = "true";
                        else pParam->value = "false";
                    }
                    else
                        pParam->value = chitems[chsel];
                }
                    
                chScrollList.Destroy();
                refreshCDKScreen(CDKScreen);
            }

            DefWindow.Clear();
            DefWindow.AddText(CreateText("</B/29>%s:<!29!B> %s", GetTranslation("Default"), GetParamDefault(pParam)),
                              false);
            DefWindow.AddText(CreateText("</B/29>%s:<!29!B> %s", GetTranslation("Current"), GetParamValue(pParam)),
                              false);
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
    ButtonBar.Clear();
    ButtonBar.AddButton("C", "Cancel");
    ButtonBar.Draw();

    std::vector<char *> askstr;
    char *dbuttons[2] = { GetTranslation("Continue"), GetTranslation("Exit program") };
    
    if (InstallInfo.dest_dir_type == DEST_SELECT)
    {
        askstr.push_back(CreateText(GetTranslation("This will install %s to the following directory:"),
                         InstallInfo.program_name.c_str()));
        askstr.push_back(MakeCString(InstallInfo.dest_dir));
        askstr.push_back(GetTranslation("Continue?"));
    }
    else
    {
        askstr.push_back(CreateText(GetTranslation("This will install %s"), InstallInfo.program_name.c_str()));
        askstr.push_back(GetTranslation("Continue?"));
    }

    CCDKDialog Diag(CDKScreen, CENTER, CENTER, &askstr[0], askstr.size(), dbuttons, 2);
    Diag.SetBgColor(26);
        
    int sel = Diag.Activate();
    
    Diag.Destroy();
    refreshCDKScreen(CDKScreen);
                
    if (sel == 1)
        return false;
    
    CCDKSWindow InstallOutput(CDKScreen, 0, 6, GetDefaultHeight()-5, -1,
                              CreateText("<C></29/B>%s", GetTranslation("Install output")), 2000);
    InstallOutput.SetBgColor(5);
    nodelay(WindowOf(InstallOutput.GetSWin()), true); // Make sure input doesn't block

    const int maxx = getmaxx(InstallOutput.GetSWin()->win);

    CCDKSWindow ProggWindow(CDKScreen, 0, 2, 5, maxx, NULL, 4);
    ProggWindow.SetBgColor(5);
    ProggWindow.AddText("");
    ProggWindow.AddText(CreateText("</B/29>%s:<!29!B>", GetTranslation("Status")));
    ProggWindow.AddText(CreateText("%s (1/%d)", GetTranslation("Extracting files"), InstallInfo.command_entries.size()+1),
                        true, BOTTOM, 24);
    
    CCDKHistogram ProgressBar(CDKScreen, 25, 3, 1, maxx-29, HORIZONTAL,
                              CreateText("<C></29/B>%s", GetTranslation("Progress")), false);
    ProgressBar.SetBgColor(5);
    ProgressBar.SetHistogram(vPERCENT, TOP, 0, 100, 0, COLOR_PAIR (24) | A_REVERSE | ' ', A_BOLD);
    
    setCDKSwindowLLChar(ProggWindow.GetSWin(), ACS_LTEE);
    setCDKSwindowLRChar(ProggWindow.GetSWin(), ACS_RTEE);
    
    InstallOutput.Draw();
    ProggWindow.Draw();
    
    // Check if we need root access
    char *passwd = NULL;
    LIBSU::CLibSU SuHandler;
    SuHandler.SetUser("root");
    SuHandler.SetTerminalOutput(false);

    bool askpass = false;
    
    for (std::list<command_entry_s *>::iterator it=InstallInfo.command_entries.begin();
        it!=InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->need_root != NO_ROOT)
        {
            // Command may need root permission, check if it is so
            if ((*it)->need_root == DEPENDED_ROOT)
            {
                param_entry_s *p = GetParamByVar((*it)->dep_param);
                if (p && !WriteAccess(p->value))
                {
                    (*it)->need_root = NEED_ROOT;
                    if (!askpass) askpass = true;
                }
            }
            else if (!askpass) askpass = true;
        }
    }

    // Ask root password if one of the command entries need root access and root isn't passwordless
    if (askpass && SuHandler.NeedPassword())
    {
        CCDKEntry entry(CDKScreen, CENTER, CENTER, CreateText("%s\n%s",
                        GetTranslation("This installation requires root(administrator) privileges in order to continue"),
                        GetTranslation("Please enter the password of the root user")), "", 60, 0, 256, vHMIXED);
        entry.SetHiddenChar('*');
        entry.SetBgColor(26);

        while(1)
        {
            char *sz = entry.Activate();

            if ((entry.ExitType() != vNORMAL) || !sz)
            {
                if (YesNoBox(CreateText("%s\n%s", GetTranslation("Root access is required to continue"),
                    GetTranslation("Abort installation?"))))
                    EndProg();
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
                if (SuHandler.GetError() == LIBSU::CLibSU::SU_ERROR_INCORRECTPASS)
                {
                    WarningBox("%s\n%s", GetTranslation("Incorrect password given for root user"),
                                GetTranslation("Please retype"));
                }
                else
                {
                    throwerror(true, "%s\n%s", GetTranslation("Could not use su to gain root access"),
                               GetTranslation("Make sure you can use su(adding the current user to the wheel group may help)"));
                }
            }
        }
        // Restore screen
        entry.Destroy();
        refreshCDKScreen(CDKScreen);
    }

    short percent = 0;
    
#if 0 // UNDONE: Need a proper way to extract files to a dir not owned by the current user
    if (!WriteAccess(InstallInfo.dest_dir) != 0)
    {
        key_t key = 32000;
        int memid = shmget(key, 1, IPC_CREAT | 0600);
        char *sharedmem;

        if (memid < 0) throwerror(true, "Could not allocate shared memory");

        sharedmem = (char *)shmat(memid, NULL, 0);

        if (sharedmem == (char *)-1) throwerror(true, "Could not read shared memory");

        *sharedmem = 1;

        void *Data[2] = { &InstallOutput, &ProgressBar };
        SuHandler.SetThinkFunc(ExtrThinkFunc, sharedmem);
        SuHandler.SetOutputFunc(PrintExtrOutput, &Data);
        SuHandler.SetCommand("extracter " + InstallInfo.own_dir);

        if (!SuHandler.ExecuteCommand(passwd))
            throwerror(true, "%s\n%s", GetTranslation("Error: Could not use extracter utility"), SuHandler.GetErrorMsgC());
    }
    else
#endif
    {
        while(percent<100)
        {
            std::string curfile;
            percent = ExtractArchive(curfile);
            
            InstallOutput.AddText("Extracting file: " + curfile, false);
            if (percent==100) InstallOutput.AddText("Done!", false);
            else if (percent==-1) throwerror(true, "Error during extracting files");
    
            ProgressBar.SetValue(0, 100, percent/(1+InstallInfo.command_entries.size()));
            ProgressBar.Draw();
    
            chtype input = getch();
            if (input == 'c')
            {
                if (YesNoBox("%s\n%s", GetTranslation("This will abort the installation"), GetTranslation("Are you sure?")))
                {
                    CleanPasswdString(passwd);
                    EndProg();
                }
            }
        }
    }

    SuHandler.SetThinkFunc(InstThinkFunc, passwd);
    SuHandler.SetOutputFunc(PrintInstOutput, &InstallOutput);

    percent = 100/(1+InstallInfo.command_entries.size()); // Convert to overall progress
    
    short step = 2; // Not 1, because extracting files is also a step
    for (std::list<command_entry_s*>::iterator it=InstallInfo.command_entries.begin();
         it!=InstallInfo.command_entries.end(); it++, step++)
    {
        if ((*it)->command.empty()) continue;

        ProggWindow.Clear();
        ProggWindow.AddText("");
        ProggWindow.AddText(CreateText("</B/29>%s:<!29!B>", GetTranslation("Status")));
        ProggWindow.AddText(CreateText("%s (%d/%d)", GetTranslation((*it)->description.c_str()), step,
                            InstallInfo.command_entries.size()+1), true, BOTTOM, 24);

        ProgressBar.Draw();
        
        std::string command = (*it)->command + " " + GetParameters(*it);
        InstallOutput.AddText("");
        InstallOutput.AddText(CreateText("Execute: %s", command.c_str()));
        InstallOutput.AddText("");
        InstallOutput.AddText("");
        
        if ((*it)->need_root == NEED_ROOT)
        {
            SuHandler.SetPath((*it)->path.c_str());
            SuHandler.SetCommand(command);
            if (!SuHandler.ExecuteCommand(passwd))
            {
                if ((*it)->exit_on_failure)
                {
                    CleanPasswdString(passwd);
                    throwerror(true, "%s\n('%s')", GetTranslation("Failed to execute install command"),
                               SuHandler.GetErrorMsgC());
                }
            }
        }
        else
        {
            // Redirect stderr to stdout, so that errors will be displayed too
            command += " 2>&1";
            
            setenv("PATH", (*it)->path.c_str(), 1);
            FILE *pipe = popen(command.c_str(), "r");
            char term[1024];
            if (pipe)
            {
                while (fgets(term, sizeof(term), pipe))
                {
                    InstallOutput.AddText(term);

                    chtype input = getch();
                    if (input == 'c') /*injectCDKSwindow(InstallOutput.GetSWin(), input);*/
                    {
                        if (YesNoBox("%s\n%s\n%s", GetTranslation("Install commands are still running"),
                                                   GetTranslation("If you abort now this may lead to a broken installation"),
                                                   GetTranslation("Are you sure?")))
                        {
                            CleanPasswdString(passwd);
                            EndProg();
                        }
                    }
                }
                
                // Check if command exitted normally and close pipe
                int state = pclose(pipe);
                if (!WIFEXITED(state) || (WEXITSTATUS(state) == 127)) // SH returns 127 if command execution failes
                {
                    if ((*it)->exit_on_failure)
                    {
                        CleanPasswdString(passwd);
                        throwerror(true, "Failed to execute install command");
                    }
                }
            }
            else
            {
                CleanPasswdString(passwd);
                throwerror(true, "Could not execute installation commands (could not open pipe)");
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
    
    CleanPasswdString(passwd);
    
    ButtonBar.Clear();
    ButtonBar.AddButton("Arrows", "Scroll install output");
    ButtonBar.AddButton("Enter", (FileExists(InstallInfo.own_dir + "/config/finish")) ? "Continue" : "Finish"); // HACK
    ButtonBar.Draw();

    WarningBox("Installation of %s complete!", InstallInfo.program_name.c_str());

    InstallOutput.Activate();
    return (InstallOutput.ExitType() == vNORMAL);
}

bool FinishInstall()
{
    char *file = CreateText("%s/config/finish", InstallInfo.own_dir.c_str());
    if (FileExists(file))
    {
        char *title = CreateText("<C></B/29>%s<!29!B>", GetTranslation("Please read the following text"));
        char *buttons[1] = { GetTranslation("OK") };
        ViewFile(file, buttons, 1, title);
        return true;
    }
    
    return true;
}
