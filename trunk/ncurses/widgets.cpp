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

// CDK Label Wrapper

CCDKLabel::CCDKLabel(CDKSCREEN *pScreen, int x, int y, char **msg, int count, bool box,
                                   bool shadow) : CBaseCDKWidget(box)
{
    m_pLabel = newCDKLabel(pScreen, x, y, msg, count, box, shadow);
    if (!m_pLabel) throwerror(false, "Could not create text label");
}

CCDKLabel::CCDKLabel(CDKSCREEN *pScreen, int x, int y, const std::string &msg, bool box,
                     bool shadow) : CBaseCDKWidget(box)
{
    char *sz[1] = { const_cast<char*>(msg.c_str()) };
    m_pLabel = newCDKLabel(pScreen, x, y, sz, 1, box, shadow);
    if (!m_pLabel) throwerror(false, "Could not create text label");
}

CCDKLabel::CCDKLabel(CDKSCREEN *pScreen, int x, int y, const char *msg, bool box,
                     bool shadow) : CBaseCDKWidget(box)
{
    char *sz[1] = { const_cast<char*>(msg) };
    m_pLabel = newCDKLabel(pScreen, x, y, sz, 1, box, shadow);
    if (!m_pLabel) throwerror(false, "Could not create text label");
}

void CCDKLabel::Destroy(void)
{
    if (!m_pLabel) return;

    CBaseCDKWidget::Destroy();
    destroyCDKLabel(m_pLabel);
    m_pLabel = NULL;
}

// CDK Button Box Wrapper

CCDKButtonBox::CCDKButtonBox(CDKSCREEN *pScreen, int x, int y, int h, int w, char *title, int rows, int cols,
                             char **buttons, int count, chtype hlight, bool box, bool shadow) : CBaseCDKWidget(box)
{
    m_pBBox = newCDKButtonbox(pScreen, x, y, h, w, title, rows, cols, buttons, count, hlight, box, shadow);
    if (!m_pBBox) throwerror(false, "Could not create button box");
}

void CCDKButtonBox::Destroy(void)
{
    if (!m_pBBox) return;
    CBaseCDKWidget::Destroy();
    destroyCDKButtonbox(m_pBBox);
    m_pBBox = NULL;
}

// CDK Scroll Wrapper

CCDKScroll::CCDKScroll(CDKSCREEN *pScreen, int x, int y, int h, int w, int sbpos, char *title, char **list, int lcount,
                       bool box, bool numbers, bool shadow) : CBaseCDKWidget(box)
{
    m_pScroll = newCDKScroll(CDKScreen, x, y, sbpos, h, w, title, list, lcount, numbers, A_REVERSE, box, shadow);
    if (!m_pScroll) throwerror(false, "Could not create scroll window");
}

void CCDKScroll::Destroy(void)
{
    if (!m_pScroll) return;

    CBaseCDKWidget::Destroy();
    destroyCDKScroll(m_pScroll);
    m_pScroll = NULL;
}

// CDK Alphalist Wrapper

CCDKAlphaList::CCDKAlphaList(CDKSCREEN *pScreen, int x, int y, int h, int w, char *title, char *label, char **list, int count,
                             bool box, bool shadow) : CBaseCDKWidget(box)
{
    m_pAList = newCDKAlphalist(pScreen, x, y, h, w, title, label, list, count, '_', A_BLINK, box, shadow);
    if (!m_pAList) throwerror(false, "Could not create alpha list");
}

void CCDKAlphaList::Destroy()
{
    if (!m_pAList) return;

    CBaseCDKWidget::Destroy();
    destroyCDKAlphalist(m_pAList);
    m_pAList = NULL;
}


// CDK Dialog Wrapper

CCDKDialog::CCDKDialog(CDKSCREEN *pScreen, int x, int y, char **message, int mcount, char **buttons, int bcount,
                       chtype hlight, bool sep, bool box, bool shadow) : CBaseCDKWidget(box)
{
    m_pDialog = newCDKDialog(pScreen, x, y, message, mcount, buttons, bcount, hlight, sep, box, shadow);
    if (!m_pDialog) throwerror(false, "Could not create dialog window");
}

CCDKDialog::CCDKDialog(CDKSCREEN *pScreen, int x, int y, const char *message, char **buttons, int bcount,
                       chtype hlight, bool sep, bool box, bool shadow) : CBaseCDKWidget(box)
{
    //std::vector<char *> vec(1, const_cast<char *>(message));
    std::vector<char *> vec;
    MakeStringList(message, vec);
    m_pDialog = newCDKDialog(pScreen, x, y, &vec[0], vec.size(), buttons, bcount, hlight, sep, box, shadow);
    if (!m_pDialog) throwerror(false, "Could not create dialog window");
}

CCDKDialog::CCDKDialog(CDKSCREEN *pScreen, int x, int y, const std::string &message, char **buttons, int bcount,
                       chtype hlight, bool sep, bool box, bool shadow) : CBaseCDKWidget(box)
{
    //std::vector<char *> vec(1, const_cast<char *>(message.c_str()));
    std::vector<char *> vec;
    MakeStringList(message, vec);
    m_pDialog = newCDKDialog(pScreen, x, y, &vec[0], vec.size(), buttons, bcount, hlight, sep, box, shadow);
    if (!m_pDialog) throwerror(false, "Could not create dialog window");
}

void CCDKDialog::SetButtonBar()
{
    ButtonBar.Push();
    ButtonBar.AddButton("TAB", "Next button");
    ButtonBar.AddButton("ENTER", "Activate button");
    ButtonBar.AddButton("ESC", "Cancel");
    ButtonBar.Draw();
}

void CCDKDialog::Destroy()
{
    if (!m_pDialog) return;

    CBaseCDKWidget::Destroy();
    destroyCDKDialog(m_pDialog);
    m_pDialog = NULL;
}

int CCDKDialog::Activate(chtype *actions)
{
    SetButtonBar();
    int ret = activateCDKDialog(m_pDialog, actions);
    ButtonBar.Pop();
    return ret;
}

// CDK SWindow Wrapper

CCDKSWindow::CCDKSWindow(CDKSCREEN *pScreen, int x, int y, int h, int w, char *title, int slines, bool box,
                         bool shadow) : CBaseCDKWidget(box)
{
    m_pSWindow = newCDKSwindow(pScreen, x, y, h, w, title, slines, box, shadow);
    if (!m_pSWindow) throwerror(false, "Could not create window");
}

void CCDKSWindow::Destroy()
{
    if (!m_pSWindow) return;

    CBaseCDKWidget::Destroy();
    destroyCDKSwindow(m_pSWindow);
    m_pSWindow = NULL;
}

void CCDKSWindow::AddText(char *txt, bool wrap, int pos, int maxch)
{
    if (maxch == -1) maxch = m_pSWindow->boxWidth-2;
    
    if (wrap)
    {
        std::istringstream fullstrm(txt);
        std::string line;

        while (fullstrm)
        {
            if (!std::getline(fullstrm, line))
                break;
            
            if (line.length() > maxch)
            {
                while (line.length() > maxch)
                {
                    std::string::size_type strpos = line.find_last_of(" \t\r\n", maxch-1);
                    if (strpos == std::string::npos)
                        strpos = maxch-1;
                    
                    addCDKSwindow(m_pSWindow, const_cast<char *>(line.substr(0, strpos).c_str()), pos);
                    line.erase(0, strpos+1);
                }
                addCDKSwindow(m_pSWindow, const_cast<char *>(line.c_str()), pos);
            }
            else
                addCDKSwindow(m_pSWindow, const_cast<char *>(line.c_str()), pos);
        }
    }
    else
    {
        std::istringstream strstrm(txt);
        std::string line;
        while (strstrm)
        {
            if (std::getline(strstrm, line))
                addCDKSwindow(m_pSWindow, const_cast<char *>(line.c_str()), pos);
        }
    }
}

// CDK Entry Wrapper

CCDKEntry::CCDKEntry(CDKSCREEN *pScreen, int x, int y, char *title, char *label, int fwidth, int min, int max,
                     EDisplayType DispType, chtype fillch, chtype fieldattr, bool box, bool shadow) : CBaseCDKWidget(box)
{
    m_pEntry = newCDKEntry(pScreen, x, y, title, label, fieldattr, fillch, DispType, fwidth, min, max, box, shadow);
    if (!m_pEntry) throwerror(false, "Could not create input entry");
}

void CCDKEntry::Destroy()
{
    if (!m_pEntry) return;

    CBaseCDKWidget::Destroy();
    destroyCDKEntry(m_pEntry);
    m_pEntry = NULL;
}

char *CCDKEntry::Activate(chtype *actions)
{
    ButtonBar.Push();
    ButtonBar.AddButton("ENTER", "OK");
    ButtonBar.AddButton("ESC", "Cancel");
    ButtonBar.Draw();
    
    char *s = activateCDKEntry(m_pEntry, actions);
    
    ButtonBar.Pop();
    return s;
}

// CDK Entry Wrapper

CCDKHistogram::CCDKHistogram(CDKSCREEN *pScreen, int x, int y, int h, int w, int orient, char *title, bool box,
                             bool shadow) : CBaseCDKWidget(box)
{
    m_pHistogram = newCDKHistogram(pScreen, x, y, h, w, orient, title, box, shadow);
    if (!m_pHistogram) throwerror(false, "Could not create histogram");
}

void CCDKHistogram::Destroy()
{
    if (!m_pHistogram) return;

    CBaseCDKWidget::Destroy();
    destroyCDKHistogram(m_pHistogram);
    m_pHistogram = NULL;
}

void CCDKHistogram::SetHistogram(EHistogramDisplayType vtype, int statspos, int min, int max, int cur, chtype fillch,
                                 chtype statattr)
{
    setCDKHistogram(m_pHistogram, vtype, statspos, statattr, min, max, cur, fillch, m_bBox);
}

// CDK Wrapper Ends here

// Button Bar

void CButtonBar::Push()
{
    if (m_pCurrentBarEntry) m_ButtonBarEntries.push_back(m_pCurrentBarEntry);
    m_pCurrentBarEntry = new bar_entry_s;
}

void CButtonBar::Pop()
{
    if (m_pCurrentBarEntry) delete m_pCurrentBarEntry;
    if (!m_ButtonBarEntries.empty())
    {
        m_pCurrentBarEntry = m_ButtonBarEntries.back();
        m_ButtonBarEntries.pop_back();
        Draw();
    }
    else m_pCurrentBarEntry = NULL;
}

void CButtonBar::AddButton(const char *button, const char *desc)
{
    if (!m_pCurrentBarEntry) m_pCurrentBarEntry = new bar_entry_s;
    
    std::string txt = std::string(" </B/16>") + GetTranslation(button) + "<!16!B>: </B/8>" + GetTranslation(desc) + "<!8!B> ";
    const int l = strlen(button) + strlen(desc) + 4; // 4 extra chars: 3 spaces and an ':'
    if ((getmaxx(MainWin)-2) < (m_pCurrentBarEntry->iCurTextLength + l))
    {
        m_pCurrentBarEntry->Texts.push_back(MakeCString(m_pCurrentBarEntry->szCurrentText));
        m_pCurrentBarEntry->szCurrentText = txt;
        m_pCurrentBarEntry->iCurTextLength = l;
    }
    else
    {
        m_pCurrentBarEntry->szCurrentText += txt;
        m_pCurrentBarEntry->iCurTextLength += l;
    }
}

void CButtonBar::Draw()
{
    if (!m_pCurrentBarEntry) return;
    
    if (!m_pCurrentBarEntry->szCurrentText.empty())
    {
        m_pCurrentBarEntry->Texts.push_back(MakeCString(m_pCurrentBarEntry->szCurrentText));
        m_pCurrentBarEntry->szCurrentText.clear();
        m_pCurrentBarEntry->iCurTextLength = 0;
    }
    
    if (m_pCurrentBarEntry->Texts.empty()) return;

    if (m_pLabel) delete m_pLabel; // CDK has no propper way to change text...
    
    m_pLabel = new CCDKLabel(CDKScreen, CENTER, BOTTOM, &m_pCurrentBarEntry->Texts[0], m_pCurrentBarEntry->Texts.size());
    if (!m_pLabel)
        throwerror(false, "Could not create button bar");
    m_pLabel->SetBgColor(24);
    
    m_pLabel->Draw();
    refreshCDKScreen(CDKScreen);
}

// File select dialog

bool CFileDialog::ReadDir(const std::string &dir)
{
    struct dirent *dirstruct;
    struct stat filestat;
    DIR *dp;
    bool isrootdir = ((dir[0] == '/') && (dir.length()==1));

    ClearDirList();

    dp = opendir (dir.c_str());
    if (dp == 0) return false;

    while ((dirstruct = readdir (dp)) != 0)
    {
        if (lstat(dirstruct->d_name, &filestat) != 0)
            continue;
        
        if (mode2Filetype(filestat.st_mode) != 'd')
            continue; // Has to be a directory...

        if (!strcmp(dirstruct->d_name, "."))
            continue;

        if ((!strcmp(dirstruct->d_name, "..")) && isrootdir)
            continue;
        
        m_DirItems.push_back(strdup(dirstruct->d_name));
    }

    closedir (dp);
    return true;
}

void CFileDialog::UpdateCurDirText()
{
    std::string dir = m_szDestDir, prefix = std::string(GetTranslation("Current")) + ": ";
    const int maxch = (m_pFileList->GetAList()->boxWidth-2) - prefix.length();

    m_pCurDirWin->Clear();

    // Check if directory fits on screen
    if (maxch < dir.length())
    {
        std::string shortdir;
        std::string::size_type index = dir.rfind("/"), prevind = dir.length();
        while ((index != std::string::npos) && (index > 0))
        {
            if ((maxch-3) < (shortdir.length() + (prevind-index)))
            {
                if (shortdir.length()) shortdir.insert(0, "...");
                else shortdir = dir.substr(dir.length()-maxch);
                break;
            }
            shortdir.insert(0, dir.substr(index, (prevind-index)));
            prevind = index;
            index = dir.rfind("/", prevind-1);
        }
        m_pCurDirWin->AddText(prefix + shortdir);

    }
    else m_pCurDirWin->AddText(prefix + dir);
}

void CFileDialog::ClearDirList()
{
    if (!m_DirItems.empty())
    {
        for (std::vector<char *>::iterator it=m_DirItems.begin(); it!=m_DirItems.end(); it++)
            free(*it);
        m_DirItems.clear();
    }
}

bool CFileDialog::Activate()
{
    char *buttons[] = { GetTranslation("Open directory"), GetTranslation("Select directory"), GetTranslation("Cancel") };
    char label[] = "Dir: ";
    char curdir[1024];
    
    ButtonBar.Push();
    ButtonBar.AddButton("TAB", "Next button");
    ButtonBar.AddButton("ENTER", "Activate button");
    ButtonBar.AddButton("Arrows", "Navigate menu");
    ButtonBar.AddButton("C", "Create directory");
    ButtonBar.AddButton("A", "About");
    ButtonBar.AddButton("ESC", "Cancel");
    ButtonBar.Draw();

    if (!getcwd(curdir, sizeof(curdir))) throwerror(true, "Could not read current directory");
    
    if (chdir(m_szStartDir.c_str()) != 0)
        throwerror(true, "Could not open directory '%s'", m_szStartDir.c_str());

    if (!ReadDir(m_szStartDir)) throwerror(true, "Could not read directory '%s'", m_szStartDir.c_str());
    
    CCDKButtonBox ButtonBox(CDKScreen, CENTER, GetDefaultHeight(), 1, GetDefaultWidth(), 0, 1, 3, buttons, 3);
    ButtonBox.SetBgColor(5);

    if (!m_pCurDirWin && !m_pFileList)
    {
        m_pCurDirWin = new CCDKSWindow(CDKScreen, CENTER, getbegy(ButtonBox.GetBBox()->win)-2, 2, GetDefaultWidth()+1,
                                                           NULL, 1);
        m_pCurDirWin->SetBgColor(5);

        if (!m_pFileList)
        {
            m_pFileList = new CCDKAlphaList(CDKScreen, CENTER, 2, getbegy(m_pCurDirWin->GetSWin()->win)-1, GetDefaultWidth()+1,
                                            const_cast<char*>(m_szTitle.c_str()), label, &m_DirItems[0], m_DirItems.size());
            m_pFileList->SetBgColor(5);
            setCDKEntryPreProcess(m_pFileList->GetAList()->entryField, CreateDirCB, this);
    
            //m_pFileList->GetAList()->entryField->dispType = vVIEWONLY;  // HACK: Disable backspace
        }
    
        setCDKAlphalistLLChar(m_pFileList->GetAList(), ACS_LTEE);
        setCDKAlphalistLRChar(m_pFileList->GetAList(), ACS_RTEE);
        setCDKLabelLLChar(m_pCurDirWin->GetSWin(), ACS_LTEE);
        setCDKLabelLRChar(m_pCurDirWin->GetSWin(), ACS_RTEE);
        setCDKButtonboxULChar(ButtonBox.GetBBox(), ACS_LTEE);
        setCDKButtonboxURChar(ButtonBox.GetBBox(), ACS_RTEE);
    }
    
    m_pFileList->Draw();
    m_pCurDirWin->Draw();
    ButtonBox.Draw();
    
    m_pFileList->Bind(KEY_TAB, SwitchButtonK, ButtonBox.GetBBox()); // Pas TAB through ButtonBox

    m_szDestDir = m_szStartDir;
    UpdateCurDirText();
    
    while(true)
    {
        // HACK: Give textbox content
        setCDKEntryValue(m_pFileList->GetAList()->entryField,
                         chtype2Char(m_pFileList->GetAList()->scrollField->item[m_pFileList->GetAList()->scrollField->currentItem]));

        char *selection = m_pFileList->Activate();

        if ((m_pFileList->ExitType() != vNORMAL) || (ButtonBox.GetCurrent() == 2)) break;
        if (!selection || !selection[0]) continue;

        if (ButtonBox.GetCurrent() == 1)
        {
            if (m_bAskWAccess && !WriteAccess(m_szDestDir))
            {
                char *dbuttons[2] = { GetTranslation("Continue as root"), GetTranslation("Choose another directory") };
                CCDKDialog Diag(CDKScreen, CENTER, CENTER,
                                GetTranslation("You don't have write permissions for this directory.\n"
                                        "The files can be extracted as the root user,\n"
                                        "but you'll need to enter the root password for this later."), dbuttons, 2);
                Diag.SetBgColor(26);
        
                int sel = Diag.Activate();
    
                Diag.Destroy();
                refreshCDKScreen(CDKScreen);
                if (sel)
                    continue;
            }
            break;
        }
        
        if (!FileExists(selection))
        {
            if (YesNoBox(GetTranslation("Directory does not exist\nDo you want to create it?")))
            {
                if (mkdir(selection, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) != 0)
                {
                    WarningBox("%s\n%.75s\n%.75s", GetTranslation("Could not create directory"), selection,
                               strerror(errno));
                    continue;
                }
            }
            else
                continue;
        }
        
        UpdateFileList(selection);
    }
    
    bool success = ((m_pFileList->ExitType() != vESCAPE_HIT) && (ButtonBox.GetCurrent() == 1));
    
    ButtonBar.Pop();

    if (m_bRestoreDir)
        chdir(curdir); // Return back to original directory
    
    return success;
}

bool CFileDialog::UpdateFileList(const char *dir)
{
    std::string newdir = m_szDestDir;
    
    newdir += "/";
    newdir += dir;
    if (chdir(newdir.c_str()))
    {
        WarningBox("%s\n%s\n%s", GetTranslation("Could not change to directory"), newdir.c_str(), strerror(errno));
        return false;
    }
    
    char tmp[1024];
    if (getcwd(tmp, sizeof(tmp))) m_szDestDir = tmp;
    else { WarningBox("Could not read current directory"); return false; }

    // Read contents of directory
    if (!ReadDir(m_szDestDir)) { WarningBox("Could not read directory"); return false; }
    
    m_pFileList->SetContent(&m_DirItems[0], m_DirItems.size());
    m_pFileList->Draw();
    
    UpdateCurDirText();

    // HACK: Give textbox content
    setCDKEntryValue(m_pFileList->GetAList()->entryField,
                     chtype2Char(m_pFileList->GetAList()->scrollField->item[m_pFileList->GetAList()->scrollField->currentItem]));
    return true;
}

void CFileDialog::Destroy(void)
{
    delete m_pFileList;
    delete m_pCurDirWin;
    m_pFileList = NULL;
    m_pCurDirWin = NULL;
    ClearDirList();
}

int CFileDialog::CreateDirCB(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED, void *clientData, chtype key)
{
    if (key != 'c')
    {
        if (key == 'a') // HACK
        {
            ShowAboutK(cdktype, object, clientData, key);
            return false;
        }
        return true;
    }
    
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

    if (mkdir(newdir, dirMode) != 0)
    {
        WarningBox("%s\n%.75s\n%.75s", GetTranslation("Could not create directory"), newdir, strerror(errno));
        return false;
    }

    CFileDialog *FileList = (CFileDialog *)clientData;
    FileList->UpdateFileList(newdir);
    return false;
}

#endif

#include "ncurses.h"

#include <sstream>

bool CWidget::HandleKeyPre(chtype ch)
{
    /*for (std::list<CWidget *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
        if ((*it)->HandleKeyPre(ch))
    return true;*/
    
    //debugline("PreKey: %d", ch);
    
    if (!m_ChildList.empty())
        return (*m_FocusedChild)->HandleKeyPre(ch);
    
    return false;
}

bool CWidget::HandleKeyPost(chtype ch)
{
    if (ch == 9)
        SetNextWidget();
    else if (ch == KEY_BTAB)
        SetPrevWidget();
    
    /*for (std::list<CWidget *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
    if ((*it)->HandleKeyPost(ch))
    return true;*/

    //debugline("PostKey: %d", ch);
    
    if (!m_ChildList.empty())
        return (*m_FocusedChild)->HandleKeyPost(ch);

    return false;
}

void CWidget::SetNextWidget()
{
    if (m_ChildList.size() < 2)
        return;
    
    (*m_FocusedChild)->LeaveFocus();
    
    m_FocusedChild++;
    if (m_FocusedChild == m_ChildList.end())
        m_FocusedChild = m_ChildList.begin();
    
    (*m_FocusedChild)->Focus();
}

void CWidget::SetPrevWidget()
{
    if (m_ChildList.size() < 2)
        return;

    (*m_FocusedChild)->LeaveFocus();
    
    if (m_FocusedChild == m_ChildList.begin())
        m_FocusedChild = m_ChildList.end();
    
    m_FocusedChild--;

    (*m_FocusedChild)->Focus();
}

void CWidget::Run()
{
    for (std::list<CWidget *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
        (*it)->Run();
}

void CWidgetManager::Run()
{
    while (true)
    {
        chtype ch = getch();
        if (ch != ERR)
        {
            //debugline("key: %d\n", ch);
            if (ch == CTRL('[')) // Escape pressed
                break;
            
            if (m_ChildList.size() == 1)
            {
                if (!m_ChildList.front()->HandleKeyPre(ch))
                    m_ChildList.front()->HandleKeyPost(ch);
            }
            else if (m_ChildList.size() > 1)
            {
                if (!(*m_FocusedChild)->HandleKeyPre(ch))
                    (*m_FocusedChild)->HandleKeyPost(ch);
            }
            /*
            for (std::list<CWidget *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
                (*it)->HandleKeyPre(ch);
            
            for (std::list<CWidget *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
            (*it)->HandleKeyPost(ch);*/
        }
        
        for (std::list<CWidget *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
            (*it)->Run();
    }
}

void CWidgetWindow::CenterText(const char *text, int row)
{
    if (row == -1)
        row = maxy()/2;
    
    int x = (maxx() - ::strlen(text)) / 2;
    if (x < 0)
        x = 0;
    addstr(row, x, text, width());
}

bool CWidgetPad::HandleKeyPost(chtype ch)
{
    if (CWidget::HandleKeyPost(ch))
        return true;
    
    //debugline("key in pad");

    // Modified code from NCursesPad's () operator (keep in sync!)
    
    NCursesWindow* W = Win();

    if (static_cast<NCursesWindow*>(0) != W)
    {
        int Width  = W->width();
        int Height = W->height();

        //int req = REQ_PAD_REFRESH;
        int req = driver(ch);
        //W->keypad(TRUE);
        //W->meta(TRUE);
        refresh();

        bool changed = FALSE;

        switch (req) {
            case REQ_PAD_REFRESH:
                // ================
                changed = TRUE;
                break;
            case REQ_PAD_LEFT:
                // =============
                if (min_col > 0) {
                    changed = TRUE;
                    if (min_col < m_iHGrid)
                        min_col = 0;
                    else
                        min_col -= m_iHGrid;
                }
                else
                    OnNavigationError(req);
                break;
            case REQ_PAD_RIGHT:
                // ==============
                if (min_col < (width() - Width - 1)) {
                    changed = TRUE;
                    if (min_col > (width() - Width - m_iHGrid - 1))
                        min_col = width() - Width - 1;
                    else
                        min_col += m_iHGrid;
                }
                else
                    OnNavigationError(req);
                break;
            case REQ_PAD_UP:
                // ===========
                if (min_row > 0) {
                    changed = TRUE;
                    if (min_row < m_iVGrid)
                        min_row = 0;
                    else
                        min_row -= m_iVGrid;
                }
                else
                    OnNavigationError(req);
                break;
            case REQ_PAD_DOWN:
                // =============
                if (min_row < (height() - Height - 1)) {
                    changed = TRUE;
                    if (min_row > (height() - Height - m_iVGrid - 1))
                        min_row = height() - Height - 1;
                    else
                        min_row += m_iVGrid;
                }
                else
                    OnNavigationError(req);
                break;

            default:
                OnUnknownOperation(req);
                return false;
        }

        if (changed) {
            noutrefresh();
            W->syncup();
            OnOperation(req);
            getWindow()->refresh();
        }
    }
    return true;
}

CButton::CButton(CWidgetPanel *owner, int nlines, int ncols, int begin_y, int begin_x, const char *text,
                 TCallBack func, char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel)
{
    bkgd(' '|COLOR_PAIR(4)|A_REVERSE);
    CenterText(text);
}

void CScrollbar::CalcScrollStep()
{
    int sbsize;
    
    if (m_bVertical)
        sbsize = height();
    else
        sbsize = width();
    
    int valrange = m_fMaxVal - m_fMinVal;
    
    m_fScrollStep = (float)sbsize / (float)valrange;
}

int CScrollbar::refresh()
{
    bkgd(' '|COLOR_PAIR(4)|A_REVERSE);
    
    // Calc slide position
    float fac = (m_fCurVal / m_fMaxVal);
    float posx, posy;
    
    if (m_bVertical)
    {
        posx = 0.0f;
        posy = ((float)height() - 1.0f) * fac; // - 1.0f becouse height/width starts at 1, while pos starts at 0
    }
    else
    {
        posx = ((float)width() - 1.0f) * fac;
        posy = 0.0f;
    }
    
    if (posx < 0.0f)
        posx = 0.0f;
    if (posy < 0.0f)
        posy = 0.0f;
    
    debugline("posx: %.2f posy: %.2f cur: %.2f step %.2f", posx, posy, m_fCurVal, m_fScrollStep);
    printw((int)posy, (int)posx, "+");
    return NCursesWindow::refresh();
}

void CScrollbar::Scroll(float n)
{
    m_fCurVal += n;
    
    if (m_fCurVal < m_fMinVal)
        m_fCurVal = m_fMinVal;
    else if (m_fCurVal > m_fMaxVal)
        m_fCurVal = m_fMaxVal;
}

CTextWindow::CTextWindow(CWidgetPanel *owner, int nlines, int ncols, int begin_y, int begin_x, bool wrap,
                         char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel), m_bWrap(wrap),
                         m_iCurrentLine(-1), m_iLongestLine(0)
{
    box();
    m_pTextWin = new CWidgetWindow(this, nlines-2, ncols-2, 1, 1, 'r');
    m_pVScrollbar = new CScrollbar(this, nlines-2, 1, 1, ncols-1, 0, 100, true, 'r');
    m_pHScrollbar = new CScrollbar(this, 1, ncols-2, nlines-1, 1, 0, 100, false, 'r');
}

void CTextWindow::VScroll(int n)
{
    m_pVScrollbar->Scroll(n);
    int diff = m_pVScrollbar->GetValue() - m_iCurrentLine;
    
    if (diff)
    {
        std::advance(m_CurrentLineIt, diff);
        m_iCurrentLine = m_pVScrollbar->GetValue();
        refresh();
    }
}

void CTextWindow::HScroll(int n)
{
    m_pHScrollbar->Scroll(n);
    refresh();
}

bool CTextWindow::HandleKeyPost(chtype ch)
{
    if (CWidgetWindow::HandleKeyPost(ch))
        return true;
    
    bool handled = true;
    
    switch (ch)
    {
        case KEY_LEFT:
            HScroll(-1);
            refresh();
            break;
        case KEY_RIGHT:
            HScroll(1);
            refresh();
            break;
        case KEY_UP:
            VScroll(-1);
            refresh();
            break;
        case KEY_DOWN:
            VScroll(1);
            refresh();
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;
}

void CTextWindow::SetText(std::string text)
{
    m_szText = text;
    m_FormattedText.clear();
    m_iLongestLine = 0;
    
    std::string line, txt;
    std::istringstream fullstrm(m_szText);
    
    while (fullstrm)
    {
        if (!std::getline(fullstrm, line))
            break;
            
        if (m_bWrap)
        {
            while (line.length() > m_pTextWin->width())
            {
                std::string::size_type strpos = line.find_last_of(" \t\r\n", m_pTextWin->width()-1);
                if (strpos == std::string::npos)
                    strpos = m_pTextWin->width()-1;
                
                txt = line.substr(0, strpos);
                m_FormattedText.push_back(txt.c_str());
                
                if (txt.length() > m_iLongestLine)
                    m_iLongestLine = txt.length();
                
                line.erase(0, strpos+1);
            }
            
            m_FormattedText.push_back(line.c_str());
            if (line.length() > m_iLongestLine)
                m_iLongestLine = line.length();
        }
        else
        {
            m_FormattedText.push_back(line.c_str());
            if (line.length() > m_iLongestLine)
                m_iLongestLine = line.length();
        }
    }
    
    if (!m_FormattedText.empty())
    {
        int h = m_FormattedText.size() - m_pTextWin->height();
        if (h < 0)
            h = 0;
        
        m_pVScrollbar->SetMinMax(0, h);
        
        if (m_iLongestLine > m_pTextWin->width())
            m_pHScrollbar->SetMinMax(0, (m_iLongestLine - m_pTextWin->width()));
        
        m_iCurrentLine = 0;
        m_CurrentLineIt = m_FormattedText.begin();
    }
}

void CTextWindow::AddText(std::string text)
{
    m_szText += text;
    
    // HACK
    if (!m_FormattedText.empty() && m_FormattedText.back()[m_FormattedText.back().length()-1] != '\n')
    {
        text = m_FormattedText.back() + text;
        //if (m_CurrentLineIt == m_FormattedText.)
        m_FormattedText.pop_back();
    }
    
    std::string line, txt;
    std::istringstream fullstrm(text);
    
    while (fullstrm)
    {
        if (!std::getline(fullstrm, line))
            break;
            
        if (m_bWrap)
        {
            while (line.length() > m_pTextWin->width())
            {
                std::string::size_type strpos = line.find_last_of(" \t\r\n", m_pTextWin->width()-1);
                if (strpos == std::string::npos)
                    strpos = m_pTextWin->width()-1;
                
                txt = line.substr(0, strpos);
                m_FormattedText.push_back(txt.c_str());
                
                if (txt.length() > m_iLongestLine)
                    m_iLongestLine = txt.length();
                
                line.erase(0, strpos+1);
            }
            
            m_FormattedText.push_back(line.c_str());
            if (line.length() > m_iLongestLine)
                m_iLongestLine = line.length();
        }
        else
        {
            m_FormattedText.push_back(line.c_str());
            if (line.length() > m_iLongestLine)
                m_iLongestLine = line.length();
        }
    }
    
    if (!m_FormattedText.empty())
    {
        int h = m_FormattedText.size() - m_pTextWin->height();
        if (h < 0)
            h = 0;
        
        m_pVScrollbar->SetMinMax(0, h);
        
        if (m_iLongestLine > m_pTextWin->width())
            m_pHScrollbar->SetMinMax(0, (m_iLongestLine - m_pTextWin->width()));
        
        //if (m_iCurrentLine == -1)
        {
            m_iCurrentLine = 0;
            m_CurrentLineIt = m_FormattedText.begin();
        }
    }
}

int CTextWindow::refresh()
{
    int lines = 0; // Printed lines
        
    clear();
    
    for(std::list<std::string>::iterator it=m_CurrentLineIt; ((it!=m_FormattedText.end()) && (lines<m_pTextWin->height()));
        it++)
    {
        m_pTextWin->printw(lines, 0, it->substr(m_pHScrollbar->GetValue()).c_str());
        lines++;
    }
    
    box();
     
    int ret = CWidgetWindow::refresh();
    m_pTextWin->refresh();
    
    if (m_FormattedText.size() > m_pTextWin->height())
        m_pVScrollbar->refresh(); // Box made it disappear, redraw when needed
    
    if (!m_bWrap && (m_iLongestLine > m_pTextWin->width()))
        m_pHScrollbar->refresh();
    
    return ret;
}
