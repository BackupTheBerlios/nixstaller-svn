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

#ifdef REMOVE_ME_AFTER_NEW_FRONTEND_IS_DONE
#include "ncurs.h"

void EndProg(bool err)
{
    if (CDKScreen)
    {
        ButtonBar.Destroy();
        destroyCDKScreen(CDKScreen);
        endCDK();
    }

    MainEnd();
    exit((err) ? EXIT_FAILURE : EXIT_SUCCESS);
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

    if (!dialog) { fprintf(stderr, GetTranslation("Error: %s"), txt); fprintf(stderr, "\n"); }
    EndProg(true);
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

int SwitchButtonK(EObjectType cdktype, void *object, void *clientData, chtype key)
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
    std::vector<char *> *items = ((std::vector<char *> *)pData[3]);

    int cur = pScroll->GetCurrent();
    
    if ((key == KEY_DOWN) && ((cur+1) < items->size())) cur++;
    else if ((key == KEY_UP) && (cur > 0)) cur--;

    param_entry_s *pParam = GetParamByName(items->at(cur));
    
    pDescWin->Clear();
    pDescWin->AddText(GetTranslation(pParam->description));
    
    pDefWin->Clear();
    pDefWin->AddText(CreateText("</B/29>%s:<!29!B> %s", GetTranslation("Default"), GetParamDefault(pParam)), false);
    pDefWin->AddText(CreateText("</B/29>%s:<!29!B> %s", GetTranslation("Current"), GetParamValue(pParam)), false);
    
    return true;
}

int ViewFile(char *file, char **buttons, int buttoncount, char *title, bool showabout, bool showexit)
{
    char **info = NULL;

    int lines = CDKreadFile(file, &info);
    if (lines == -1)
        return NO_FILE;

    CDKVIEWER *Viewer = newCDKViewer(CDKScreen, CENTER, 2, GetDefaultHeight(), GetDefaultWidth(), buttons, buttoncount,
                                     A_REVERSE, true, false);

    if (Viewer == NULL)
        throwerror(false, "Can't create text viewer");

    ButtonBar.Push();
    if (buttoncount > 1) ButtonBar.AddButton("TAB", "Next button");
    ButtonBar.AddButton("ENTER", "Activate button");
    ButtonBar.AddButton("Arrows", "Scroll text");
    if (showabout) ButtonBar.AddButton("A", "About");
    if (showexit) ButtonBar.AddButton("ESC", "Exit program");
    else ButtonBar.AddButton("ESC", "Close");
    ButtonBar.Draw();

    setCDKViewerBackgroundColor(Viewer, "</B/5");
    bindCDKObject(vVIEWER, Viewer, 'a', ShowAboutK, NULL);
    setCDKViewer(Viewer, title, info, lines, A_REVERSE, true, true, true);
    
    int selected = activateCDKViewer(Viewer, NULL);
    int ret = (Viewer->exitType == vNORMAL) ? selected : ESCAPE;

    CDKfreeStrings(info);
    setCDKViewerBackgroundColor(Viewer, "!5!B");
    destroyCDKViewer(Viewer);
    ButtonBar.Pop();
    refreshCDKScreen(CDKScreen);
    return ret;
}

void WarningBox(const char *msg, ...)
{
    std::vector<char *> message;
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
        message.push_back(MakeCString(unwrapped.substr(prevind, (index-prevind))));
        prevind = index+1;
        index = unwrapped.find("\n", prevind);
    }
    message.push_back(MakeCString(unwrapped.substr(prevind, index)));
    
    CCDKDialog Diag(CDKScreen, CENTER, CENTER, &message[0], message.size(), buttons, 1);
    Diag.SetBgColor(26);
    Diag.Activate();
    Diag.Destroy();
    refreshCDKScreen(CDKScreen);
}

bool YesNoBox(const char *msg, ...)
{
    std::vector<char *> message;
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
        message.push_back(MakeCString(unwrapped.substr(prevind, (index-prevind))));
        prevind = index+1;
        index = unwrapped.find("\n", prevind);
    }
    message.push_back(MakeCString(unwrapped.substr(prevind, index)));
    
    CCDKDialog Diag(CDKScreen, CENTER, CENTER, &message[0], message.size(), buttons, 2);
    Diag.SetBgColor(26);
    bool yes = ((Diag.Activate() == 0) && (Diag.ExitType() == vNORMAL));
    Diag.Destroy();
    refreshCDKScreen(CDKScreen);
    return yes;
}

void InstThinkFunc(void *p)
{
    // Key mode is unblocked in InstallFiles()
    chtype input = getch();
    if (input == 'c')
    {
        if (YesNoBox(GetTranslation("Install commands are still running\n"
            "If you abort now this may lead to a broken installation\n"
            "Are you sure?")))
        {
            CleanPasswdString((char *)p);
            EndProg();
        }
    }
}

int ShowAboutK(EObjectType cdktype, void *object, void *clientData, chtype key)
{
    char *buttons[1] = { GetTranslation("OK") };
    ViewFile(CreateText("%s/about", InstallInfo.own_dir.c_str()), buttons, 1, CreateText("<C></B/29>%s<!29!B>",
             GetTranslation("About")), false, false);
    
    return true;
}
#endif

#include "ncurses.h"
#include <stack>

// stack containing addresses of buttons which are called by GenButtonCB().
// By comparing the top button from the stack we can see which button was last called.
static std::stack<CWidgetHandler *> ButtonStack;
static bool GenButtonCB(CWidgetHandler *p, CWidgetWindow *owner) { owner->Enable(false); ButtonStack.push(p); return true; };

// Simple close window callback function.
static bool CloseCB(CWidgetHandler *p, CWidgetWindow *owner) { owner->Enable(false); return true; };

int MaxX()
{
    int y, x;
    getmaxyx(stdscr, y, x);
    return x;
}

int MaxY()
{
    int y, x;
    getmaxyx(stdscr, y, x);
    return y;
}

void MessageBox(const char *msg, ...)
{
    char *text;
    va_list v;
    
    va_start(v, msg);
    vasprintf(&text, msg, v);
    va_end(v);
    
    int width = Min(35, MaxX());
    CWidgetWindow *win = new CWidgetWindow(&WidgetManager, MaxY(), width, 0, (MaxX()-width)/2);
    
    CTextLabel *label = new CTextLabel(win, 5, width-4, 2, 2, 'r');
    label->AddText(text);
    
    CButton *button = new CButton(win, 1, 10, (label->rely()+label->maxy()+2), (win->maxx()-10)/2, "OK", 'r');
    button->SetCallBack(CloseCB, win);
    
    win->resize(button->rely()+button->maxy()+2, win->width());
    win->mvwin((MaxY() - win->maxy())/2, (MaxX() - win->maxx())/2);
    
    erase();
    
    WidgetManager.Refresh();
    while(WidgetManager.Run() && win->Enabled());
    
    free(text);
}

bool YesNoBox(const char *msg, ...)
{
    char *text;
    va_list v;
    bool ret = false;
    
    va_start(v, msg);
    vasprintf(&text, msg, v);
    va_end(v);
    
    int width = Min(40, MaxX());
    CWidgetWindow *win = new CWidgetWindow(&WidgetManager, MaxY()-2, width, 2, (MaxX()-width)/2);
    
    CTextLabel *label = new CTextLabel(win, 10, width-4, 2, 2, 'r');
    label->AddText(text);
    
    CButton *buttonyes = new CButton(win, 1, 15, (label->rely()+label->maxy()+2),
                                     (win->maxx()-((2*15)+2))/2, "Yes", 'r');
    CButton *buttonno = new CButton(win, 1, 15, (label->rely()+label->maxy()+2),
                                    (buttonyes->relx()+buttonyes->maxx()+2), "No", 'r');
    buttonyes->SetCallBack(GenButtonCB, win);
    buttonno->SetCallBack(GenButtonCB, win);
    
    win->resize(buttonyes->rely()+buttonyes->maxy()+2, win->width());
    win->mvwin((MaxY() - win->maxy())/2, (MaxX() - win->maxx())/2);
    
    erase();
    WidgetManager.Refresh();
    
    while(WidgetManager.Run() && win->Enabled());
    
    if (!ButtonStack.empty())
    {
        if (ButtonStack.top() == buttonyes)
        {
            ret = true;
            ButtonStack.pop();
        }
        else if (ButtonStack.top() == buttonno)
        {
            ret = false;
            ButtonStack.pop();
        }
    }
    
    free(text);
    return ret;
}

std::string FileDialog(const char *start, const char *info, bool needw)
{
    int maxx, maxy;
    getmaxyx(stdscr, maxy, maxx);
    
    int width = Min(70, MaxX());
    CFileDialog *filedialog = new CFileDialog(&WidgetManager, MaxY()-2, width, 2, (MaxX()-width)/2, start, info, needw);
    
    WidgetManager.Refresh();
    
    while(WidgetManager.Run() && filedialog->Enabled());
    
    return *filedialog->Selection();
}
