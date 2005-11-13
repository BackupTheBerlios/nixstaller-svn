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
    if (BottomLabel) delete BottomLabel;
    if (CDKScreen) destroyCDKScreen(CDKScreen);
    
    endCDK();
    FreeStrings();
    MainEnd();
    return 0;
}

bool SelectLanguage()
{
    char title[] = "<C></B/29>Please select a language<!29!B>";
    CCharListHelper LangItems;
    
    for (std::list<char*>::iterator p=InstallInfo.languages.begin();p!=InstallInfo.languages.end();p++)
        LangItems.AddItem(*p);

    CCDKScroll ScrollList(CDKScreen, CENTER, CENTER, DEFAULT_HEIGHT, DEFAULT_WIDTH, RIGHT, title, LangItems,
                          LangItems.Count());
    ScrollList.SetBgColor(5);
    
    int selection = ScrollList.Activate();
    
    bool success = false;
    if (ScrollList.ExitType() == vNORMAL)
    {
        std::list<char *>::iterator it = InstallInfo.languages.begin();
        advance(it, selection);
        InstallInfo.cur_lang = *it;
        success = ReadLang();
    }
        
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
    if (!InstallInfo.need_file_dialog) return true;
    
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

    if (chdir(InstallInfo.dest_dir) != 0)
        throwerror("Couldn't open directory '%s'", InstallInfo.dest_dir);

    int count = ReadDir(InstallInfo.dest_dir, &item);

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
        
        FileList.SetContent(item, count);
        FileList.Draw();
    }
    
    bool success = ((FileList.ExitType() != vESCAPE_HIT) && (ButtonBox.GetCurrent() == 1));
    
    CDKfreeStrings(item);

    return success;
}

bool ConfParams()
{
    char *title = CreateText("<C></B/29>%s<!29!B>", GetTranslation("Configuring parameters"));
    char *buttons[3] = { GetTranslation("Edit parameter"), GetTranslation("Continue install"), GetTranslation("Cancel") };
    std::string firstdesc, firstdef;
    short s=0;
    CCharListHelper ParamItems;
    
    for (std::list<command_entry_s *>::iterator p=InstallInfo.command_entries.begin();p!=InstallInfo.command_entries.end();
         p++)
    {
        for (std::map<std::string, command_entry_s::param_entry_s *>::iterator p2=(*p)->parameter_entries.begin();
             p2!=(*p)->parameter_entries.end();p2++)
        {
            if (firstdesc.empty())
            {
                firstdesc = p2->second->description;
                firstdef = p2->second->defaultval;
            }
            ParamItems.AddItem(p2->first);
        }
    }
    
    CCDKButtonBox ButtonBox(CDKScreen, CENTER, DEFAULT_HEIGHT, 1, 68, 0, 1, 3, buttons, 3);
    ButtonBox.SetBgColor(5);

    CCDKScroll ScrollList(CDKScreen, getbegx(ButtonBox.GetBBox()->win), 2, DEFAULT_HEIGHT-1, 35, RIGHT, title, ParamItems,
                          ParamItems.Count());
    ScrollList.SetBgColor(5);
    
    CCDKSWindow DescWindow(CDKScreen, getbegx(ButtonBox.GetBBox()->win)+35, 2, 5, 34, CreateText("<C></B/29>%s<!29!B>",
                           GetTranslation("Description")), 4);
    DescWindow.SetBgColor(5);
    DescWindow.AddText(firstdesc);
    
    CCDKSWindow DefWindow(CDKScreen, getbegx(ButtonBox.GetBBox()->win)+35, 8, 3, 34, CreateText("<C></B/29>%s<!29!B>",
                          GetTranslation("Default")), 4);
    DefWindow.SetBgColor(5);
    DefWindow.AddText(firstdef, false);

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
    
    std::pair<CDKSWINDOW *, CDKSWINDOW *> pair(DescWindow.GetSWin(), DefWindow.GetSWin());
    setCDKScrollPreProcess(ScrollList.GetScroll(), ScrollParamMenuK, &pair);

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
                std::map<std::string, command_entry_s::param_entry_s *>::iterator p2;
                p2 = (*p)->parameter_entries.find(ParamItems[selection]);
                if (p2 != (*p)->parameter_entries.end())
                {
                    if (p2->second->param_type == command_entry_s::param_entry_s::PTYPE_STRING)
                    {
                        CCDKEntry entry(CDKScreen, CENTER, CENTER, GetTranslation("Please enter new value"), "", 40, 0, 256);
                        entry.SetBgColor(26);
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
                        if (p2->second->param_type == command_entry_s::param_entry_s::PTYPE_BOOL)
                        {
                            chitems.AddItem("Disable");
                            chitems.AddItem("Enable");
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
                            p2->second->value = chitems[chsel];
                            
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
    
    short percent = 0;
    while(percent<100)
    {
        char curfile[256], text[300];
        percent = ExtractArchive(curfile);
        
        sprintf(text, "Extracting file: %s", curfile);
        InstallOutput.AddText(text, false);
        if (percent==100) InstallOutput.AddText("Done!", false);
        else if (percent==-1) return false;
        InstallOutput.Draw();
        
        ProgressBar.SetValue(0, 100, percent);
        ProgressBar.Draw();
    }
    
    if (InstallInfo.install_type == INST_COMPILE) // More stuff to come...
    {
        ProgressBar.SetValue(0, 100, 0);
        ProgressBar.Draw();
        percent = 0;
        
        CLibSU SuHandler;
        bool needrootpw = SuHandler.NeedPassword();
        for (std::list<command_entry_s*>::iterator it=InstallInfo.command_entries.begin();
             it!=InstallInfo.command_entries.end(); it++)
        {
            std::string command = (*it)->command + " " + GetParameters(*it);
            InstallOutput.AddText(CreateText("\nExecute: %s\n\n", command.c_str()));
            
            if (needrootpw && (*it)->need_root)
            {
            }
            else
            {
                FILE *pPipe = popen((*it)->command.c_str(), "r");
                if (pPipe)
                {
                    char buf[1024];
                    while(fgets(buf, sizeof(buf), pPipe))
                    {
                        InstallOutput.AddText(buf);
                    }
                    fclose(pPipe);
                }
                else
                {
                    // UNDONE
                }
            }
            percent += (1.0f/(float)InstallInfo.command_entries.size())*100.0f;
            ProgressBar.SetValue(0, 100, percent);
            ProgressBar.Draw();
        }
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
