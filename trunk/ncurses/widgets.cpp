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

#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>

// -------------------------------------
// Widget handler class
// -------------------------------------

CWidgetHandler::~CWidgetHandler()
{
    for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!= m_ChildList.end(); it++)
        delete *it;
}

bool CWidgetHandler::HandleKey(chtype ch)
{
    if (m_FocusedChild != m_ChildList.end())
    {
        if ((ch == 9) && SetNextWidget())
            return true;
        
        if ((ch == KEY_BTAB) && SetPrevWidget())
            return true;
        
        return ((*m_FocusedChild)->HandleKey(ch));
    }
    
    return false;
}

bool CWidgetHandler::SetNextWidget()
{
    if (m_ChildList.size() < 2)
    {
        bool ret = (m_FocusedChild != m_ChildList.begin());
        m_FocusedChild = m_ChildList.begin();
        return ret;
    }
    else if (m_ChildList.empty())
    {
        m_FocusedChild = m_ChildList.end();
        return false;
    }
    
    std::list<CWidgetWindow *>::iterator prev;
    if (m_FocusedChild != m_ChildList.end())
        prev = m_FocusedChild;
    else
        prev = m_ChildList.begin();
    
    for (m_FocusedChild++; m_FocusedChild != prev; m_FocusedChild++)
    {
        if (m_FocusedChild == m_ChildList.end())
            m_FocusedChild = m_ChildList.begin();
        
        if ((*m_FocusedChild)->CanFocus() && (*m_FocusedChild)->Enabled() && !(*m_FocusedChild)->m_bDeleteMe)
        {
            (*prev)->LeaveFocus();
            (*m_FocusedChild)->Focus();
            return true;
        }
    }
    
    return false;
}

bool CWidgetHandler::SetPrevWidget()
{
    if (m_ChildList.size() < 2)
    {
        bool ret = (m_FocusedChild != m_ChildList.begin());
        m_FocusedChild = m_ChildList.begin();
        return ret;
    }
    else if (m_ChildList.empty())
    {
        m_FocusedChild = m_ChildList.end();
        return false;
    }

    std::list<CWidgetWindow *>::iterator prev;
    if (m_FocusedChild != m_ChildList.end())
        prev = m_FocusedChild;
    else
        prev = m_ChildList.begin();
    
    do
    {
        if (m_FocusedChild == m_ChildList.begin())
            m_FocusedChild = m_ChildList.end();
    
        m_FocusedChild--;
        
        if ((*m_FocusedChild)->CanFocus() && (*m_FocusedChild)->Enabled() && !(*m_FocusedChild)->m_bDeleteMe)
        {
            (*prev)->LeaveFocus();
            (*m_FocusedChild)->Focus();
            return true;
        }
    }
    while (m_FocusedChild != prev);

    return false;
}

void CWidgetHandler::Focus()
{
    m_bFocused = true;
    
    if (m_FocusedChild != m_ChildList.end())
        (*m_FocusedChild)->Focus();
}

void CWidgetHandler::LeaveFocus()
{
    m_bFocused = false;
    
    if (m_FocusedChild != m_ChildList.end())
        (*m_FocusedChild)->LeaveFocus();
}

void CWidgetHandler::PushEvent(int type)
{
    CWidgetHandler *owner = m_pOwner;
    
    while (owner && !owner->HandleEvent(this, type))
        owner = owner->m_pOwner;
}

void CWidgetHandler::AddChild(CWidgetWindow *p)
{
    if (m_FocusedChild != m_ChildList.end())
    {
        if (p->CanFocus() && p->Enabled() && ((*m_FocusedChild)->CanFocus()) && ((*m_FocusedChild)->Enabled()))
            (*m_FocusedChild)->LeaveFocus();
    }
    
    m_ChildList.push_back(p);
    
    if (p->CanFocus() && p->Enabled())
    {
        m_FocusedChild = m_ChildList.end();
        m_FocusedChild--;
    }
    
    p->Focus();
}

void CWidgetHandler::RemoveChild(CWidgetWindow *p)
{
    if (*m_FocusedChild == p)
        SetPrevWidget();
    
    bool changed = (*m_FocusedChild != p);
    m_ChildList.remove(p);
    
    if (!changed)
    {
        // Have to do this after the child was removed from the list, otherwise it would be invalid
        m_FocusedChild = m_ChildList.end();
    }
    
    delete p;
}

void CWidgetHandler::ActivateChild(CWidgetWindow *p)
{
    if (!p->Focused())
    {
        if ((m_FocusedChild != m_ChildList.end()) && (*m_FocusedChild)->CanFocus() &&
             (*m_FocusedChild)->Enabled())
            (*m_FocusedChild)->LeaveFocus();
        p->Focus();
        m_FocusedChild = std::find(m_ChildList.begin(), m_ChildList.end(), p);
    }
}

void CWidgetHandler::Enable(bool e)
{
    m_bEnabled = e;
    
    if (!e)
    {
        // Disabled widgets shouldn't have focus
        if (*m_pOwner->m_FocusedChild == this)
            m_pOwner->SetPrevWidget();
    }
}

// -------------------------------------
// Widget manager class
// -------------------------------------

void CWidgetManager::Init()
{
    nodelay(stdscr, true);
    
    // Init all default colors
    CWidgetWindow::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CWidgetWindow::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CButton::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CButton::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CTextLabel::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    CTextLabel::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    
    CTextWindow::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CTextWindow::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CMenu::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CMenu::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CInputField::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CInputField::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    
    CProgressbar::m_cDefaultFillColors = ' ' | CWidgetWindow::GetColorPair(COLOR_BLUE, COLOR_WHITE) | A_BOLD;
    CProgressbar::m_cDefaultEmptyColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    
    CMessageBox::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CMessageBox::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CWarningBox::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_RED) | A_BOLD;
    CWarningBox::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_RED) | A_BOLD;
    
    CYesNoBox::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CYesNoBox::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CChoiceBox::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CChoiceBox::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CInputDialog::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CInputDialog::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CFileDialog::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CFileDialog::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
}

void CWidgetManager::Refresh()
{
    // Move cursor to end of display(some terminals aren't able to hide it)
    ::move(MaxY(), MaxY());
    
    ::refresh();

    for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
    {
        if ((*it)->Enabled() && !(*it)->m_bDeleteMe)
            (*it)->refresh();
    }
}

void CWidgetManager::RemoveChild(CWidgetWindow *p)
{
    p->m_bDeleteMe = true;
}

void CWidgetManager::ActivateChild(CWidgetWindow *p)
{
    if (m_ChildList.back() == p)
        return; // Already 'activated'
    
    for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
    {
        if (*it == p)
        {
            m_ChildList.erase(it);
            AddChild(p);
            Refresh();
            break;
        }
    }
}

bool CWidgetManager::Run()
{
    if (m_bQuit)
        return false;
    
    // Check for widgets that should be removed
    bool done = false, dirty = false;
    while(!done)
    {
        std::list<CWidgetWindow *>::iterator it = m_ChildList.begin();
        for (;it!=m_ChildList.end(); it++)
        {
            if ((*it)->m_bDeleteMe)
            {
                CWidgetWindow *w = *it;
                if (m_FocusedChild == it)
                    SetPrevWidget();
    
                bool changed = (m_FocusedChild != it);
                m_ChildList.erase(it);
    
                if (!changed)
                {
                    // Have to do this after the child was removed from the list, otherwise it would be invalid
                    m_FocusedChild = m_ChildList.end();
                }
    
                delete w;
                dirty = true;
                break; // Iter was removed, start over
            }
        }
        done = (it == m_ChildList.end());
    }
    
    if (dirty)
    {
        ::erase();
        Refresh();
    }

    if (m_FocusedChild != m_ChildList.end())
    {
        chtype ch = (*m_FocusedChild)->getch();
        if (ch != ERR)
        {
            if (!(*m_FocusedChild)->HandleKey(ch))
            {
                if (ch == CTRL('[')) // Escape pressed
                {
                    // Set this so that later calls will also return false. This is required incase this function
                    // wasn't called from the main app.
                    m_bQuit = true;
                    return false;
                }
            }
        }
    }
    
    return true;
}

// -------------------------------------
// Widget window class
// -------------------------------------

CWidgetWindow::ColorMapType CWidgetWindow::m_ColorPairs;
int CWidgetWindow::m_iCurColorPair = 0;
chtype CWidgetWindow::m_cDefaultFocusedColors;
chtype CWidgetWindow::m_cDefaultDefocusedColors;
    
CWidgetWindow::CWidgetWindow(CWidgetManager *owner, int nlines, int ncols, int begin_y, int begin_x, bool box,
                             chtype fcolor, chtype dfcolor) : CWidgetHandler(owner),
                                                              NCursesWindow(nlines, ncols, begin_y, begin_x),
                                                              m_bBox(box), m_sCurColor(0), m_cLLCorner(0),
                                                              m_cLRCorner(0), m_cULCorner(0), m_cURCorner(0)
{
    SetColors(fcolor, dfcolor);
    owner->AddChild(this);
}

CWidgetWindow::CWidgetWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel,
                             bool box, bool canfocus, chtype fcolor,
                             chtype dfcolor) : CWidgetHandler(owner, canfocus),
                                               NCursesWindow(*owner, nlines, ncols, begin_y, begin_x, absrel),
                                               m_bBox(box), m_sCurColor(0), m_cLLCorner(0), m_cLRCorner(0),
                                               m_cULCorner(0), m_cURCorner(0)
{
    SetColors(fcolor, dfcolor);
    owner->AddChild(this);
}

CWidgetWindow::CWidgetWindow(CWidgetManager *owner, int nlines, int ncols, int begin_y, int begin_x,
                             bool box) : CWidgetHandler(owner), NCursesWindow(nlines, ncols, begin_y, begin_x),
                                         m_bBox(box), m_sCurColor(0), m_cLLCorner(0), m_cLRCorner(0), m_cULCorner(0),
                                         m_cURCorner(0)
{
    SetColors(m_cDefaultFocusedColors, m_cDefaultDefocusedColors);
    owner->AddChild(this);
}

CWidgetWindow::CWidgetWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel,
                             bool box) : CWidgetHandler(owner),
                                         NCursesWindow(*owner, nlines, ncols, begin_y, begin_x, absrel),
                                         m_bBox(box), m_sCurColor(0), m_cLLCorner(0), m_cLRCorner(0),
                                         m_cULCorner(0), m_cURCorner(0)
{
    SetColors(m_cDefaultFocusedColors, m_cDefaultDefocusedColors);
    owner->AddChild(this);
}

unsigned CWidgetWindow::GetUnFormatLen(const std::string &str)
{
    unsigned length = str.length();
    std::string::size_type pos = 0;
    
    while ((pos = str.find("<C>", pos)) != std::string::npos)
    {
        length -= 3;
        pos += 3;
    }
    while ((pos = str.find("<col=", pos)) != std::string::npos)
    {
        std::string::size_type end = str.find(">", pos+5);
        if (end != std::string::npos)
        {
            length -= ((end - pos)+1);
            pos += ((end - pos)+1);
        }
    }
    while ((pos = str.find("</col>", pos)) != std::string::npos)
    {
        length -= 6;
        pos += 6;
    }
    while ((pos = str.find("<rev>", pos)) != std::string::npos)
    {
        length -= 5;
        pos += 5;
    }
    while ((pos = str.find("</rev>", pos)) != std::string::npos)
    {
        length -= 6;
        pos += 6;
    }
    
    return length;
}

int CWidgetWindow::refresh()
{
    Draw();

    if (m_bBox)
        Box();
    
    if (!m_szTitle.empty())
        addstr(0, (maxx()-m_szTitle.length())/2, m_szTitle.c_str());
        
    int ret = NCursesWindow::refresh();
    
    for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
    {
        if ((*it)->Enabled())
            (*it)->refresh();
    }
    
    return ret;
}

void CWidgetWindow::AddStrFormat(int y, int x, std::string ftext, int start, int n)
{
    std::string line;
    std::string::size_type strstart=0, strend=0, chars=0, len;
    
    line = ftext.substr(0, ftext.find('\n'));
    if (line.find("<C>") != std::string::npos)
    {
        int w = (n != -1) ? n : width();
        len = GetUnFormatLen(line);
        if (len < w)
            ftext.insert(0, ((w - len) / 2), ' '); // Add spaces so it centers
    }
        
    while (strstart < ftext.length())
    {
        if (ftext[strstart] == '<')
        {
            if (!ftext.compare(strstart+1, 4, "col=")) // Color tag
            {
                strstart += 5; // length of <col= == 5
                strend = ftext.find(">", strstart);
                short cpair = atoi(ftext.substr(strstart, (strend-strstart)).c_str());
                m_sCurColor = cpair;
                attron(COLOR_PAIR(cpair));
                strstart = strend+1;
            }
            else if (!ftext.compare(strstart+1, 5, "/col>")) // End color tag
            {
                strstart += 6;
                attroff(COLOR_PAIR(m_sCurColor));
            }
            else if (!ftext.compare(strstart+1, 2, "C>")) // Center tag
                strstart += 3; // Ignore, is handled earlier
            else if (!ftext.compare(strstart+1, 4, "rev>")) // Reverse color tag
            {
                strstart += 5;
                attron(A_REVERSE);
            }
            else if (!ftext.compare(strstart+1, 5, "/rev>")) // Reverse color tag
            {
                strstart += 6;
                attroff(A_REVERSE);
            }
            else
            {
                // Only print stuff when in range (but still process remaining tags)
                if (((start == -1) || (chars >= start)) && ((n == -1) || (x < n)))
                {
                    addch(y, x, ftext[strstart]);
                    
                    if (ftext[strstart] == '\n')
                    {
                        x = 0;
                        
                        unsigned pos = ftext.find('\n');
                        if (pos != std::string::npos)
                            pos -= (strstart+1);
                        
                        line = ftext.substr(strstart+1, pos);
                        if (line.find("<C>") != std::string::npos)
                        {
                            int w = (n != -1) ? n : width();
                            len = GetUnFormatLen(line);
                            if (len < w)
                                ftext.insert(strstart+1, ((w - len) / 2), ' '); // Add spaces so it centers
                        }
                    }
                    else
                        x++;
                }
                chars++;
                strstart++;
            }
        }
        else
        {
            if (((start == -1) || (chars >= start)) && ((n == -1) || (x < n)))
            {
                addch(y, x, ftext[strstart]);
                x++;
            }
            chars++;
            strstart++;
        }
    }
}

int CWidgetWindow::mvwin(int begin_y, int begin_x)
{
    int diffy = begy() - begin_y;
    int diffx = begx() - begin_x;
    
    int ret = NCursesWindow::mvwin(begin_y, begin_x);
    
    if (ret != ERR)
    {
        // Child widgets won't move without this...
        if (m_pOwner)
        {
            w->_begy = begin_y;
            w->_begx = begin_x;
        }
        
        // Move all child widgets
        for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
        {
            if ((*it)->mvwin((*it)->begy() - diffy, (*it)->begx() - diffx) == ERR)
                return ERR;
        }
    }

    return ret;
}

int CWidgetWindow::GetColorPair(int fg, int bg)
{
    if (!::has_colors())
    {
        // Non-monochrome color requested?
        if (((fg != COLOR_BLACK) && (fg != COLOR_WHITE)) ||
            ((bg != COLOR_BLACK) && (bg != COLOR_WHITE)))
            return GetColorPair(COLOR_WHITE, COLOR_BLACK);
    }
    
    ColorMapType::iterator it = m_ColorPairs.find(fg);
    if (it != m_ColorPairs.end())
    {
        std::map<int, int>::iterator it2 = it->second.find(bg);
        if (it2 != it->second.end())
            return COLOR_PAIR(it2->second);
    }
    
    m_iCurColorPair++;
    ::init_pair(m_iCurColorPair, fg, bg);
    m_ColorPairs[fg][bg] = m_iCurColorPair;
    return COLOR_PAIR(m_iCurColorPair);
}

// -------------------------------------
// Button widget class
// -------------------------------------

chtype CButton::m_cDefaultFocusedColors;
chtype CButton::m_cDefaultDefocusedColors;

CButton::CButton(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, const char *text,
                 char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel, false, true,
                 m_cDefaultFocusedColors, m_cDefaultDefocusedColors)
{
    m_szDefocusedTitle = "<C>";
    m_szDefocusedTitle += text;
    m_szFocusedTitle = "<C>< " + m_szDefocusedTitle + " >";
    m_pCurrentTitle = &m_szFocusedTitle;
}

bool CButton::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    if ((ch == KEY_ENTER) || (ch == '\n') || (ch == 'r'))
    {
        PushEvent(EVENT_CALLBACK);
        return true;
    }
    
    return false;
}

// -------------------------------------
// Scrollbar widget class
// -------------------------------------

CScrollbar::CScrollbar(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, float min, float max,
                       bool vertical, char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x,
                                                                   absrel, false, false, m_cDefaultFocusedColors,
                                                                   m_cDefaultDefocusedColors),
                                                     m_fMinVal(min), m_fMaxVal(max), m_fCurVal(min),
                                                     m_bVertical(vertical)
{
}

void CScrollbar::Draw()
{
    if (par)
        bkgd(par->getbkgd()|A_REVERSE);
    else
        bkgd(::getbkgd(stdscr)|A_REVERSE);
    
    // Calc slide position
    float fac = fabs(m_fCurVal / (m_fMaxVal-m_fMinVal));
    float posx, posy;
    
    if (m_bVertical)
    {
        posx = 0.0f;
        posy = (float)maxy() * fac;
    }
    else
    {
        posx = (float)maxx() * fac;
        posy = 0.0f;
    }
    
    if (posx < 0.0f)
        posx = 0.0f;
    if (posy < 0.0f)
        posy = 0.0f;
    
    addch((int)posy, (int)posx, ACS_CKBOARD);
}

void CScrollbar::Scroll(float n)
{
    m_fCurVal += n;
    
    if (m_fCurVal < m_fMinVal)
        m_fCurVal = m_fMinVal;
    else if (m_fCurVal > m_fMaxVal)
        m_fCurVal = m_fMaxVal;
}

// -------------------------------------
// Text label class
// -------------------------------------

chtype CTextLabel::m_cDefaultFocusedColors;
chtype CTextLabel::m_cDefaultDefocusedColors;

CTextLabel::CTextLabel(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                       char absrel) : CWidgetWindow(owner, 3, ncols, begin_y, begin_x, absrel, false,
                                                   false, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                                      m_iMaxHeight(nlines)
{
}

void CTextLabel::AddText(std::string text)
{
    unsigned lines = 0, curlines = 0;
    std::string word, line;
    std::string::size_type start=0, end;
    
    if (!m_FormattedText.empty())
        lines = curlines = m_FormattedText.size();

    if (lines >= m_iMaxHeight)
        return;
    
    do
    {
        end = text.find_first_of(" \t\n", start);
        
        if (end != std::string::npos)
            word = text.substr(start, (end-start));
        else
            word = text.substr(start);
        
        if (end != std::string::npos)
        {
            start = end;
            end = text.find_first_not_of(" \t", start+1);
            if (end != std::string::npos)
            {
                short count = ((end - start) <= (width() - start)) ? (end-start) : (width()-start);
                word += text.substr(start, count);
                start = end;
            }
            else
                start++; // Start searching on next char
        }
        
        if (m_FormattedText.empty() || (m_FormattedText.back()[m_FormattedText.back().length()-1] == '\n') ||
            ((GetUnFormatLen(m_FormattedText.back())+GetUnFormatLen(word)) > width()))
        {
            if ((lines+1) >= m_iMaxHeight)
                break; // No more space
            m_FormattedText.push_back("");
            lines++;
        }
        
        m_FormattedText.back() += word;
    }
    while((end != std::string::npos) && (lines < m_iMaxHeight));
    
    if (lines != curlines)
        resize(lines, width());
}

void CTextLabel::Draw()
{
    erase();
    
    if (!m_FormattedText.empty())
    {
        int lines = 0;
        for(std::list<std::string>::iterator it=m_FormattedText.begin(); it!=m_FormattedText.end(); it++, lines++)
            AddStrFormat(lines, 0, it->c_str(), 0, width());
    }
}

// -------------------------------------
// Text window class
// -------------------------------------

chtype CTextWindow::m_cDefaultFocusedColors;
chtype CTextWindow::m_cDefaultDefocusedColors;

CTextWindow::CTextWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, bool wrap, bool follow,
                         char absrel, bool box) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel, box,
                                                                box, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                                                  m_iLongestLine(0), m_bWrap(wrap), m_bFollow(follow),
                                                  m_iCurrentLine(-1)
{
    if (box)
        m_pTextWin = new CWidgetWindow(this, nlines-2, ncols-2, 1, 1, 'r', false); // Create a new text window
    else
        m_pTextWin = this; // Use current
    
    m_pVScrollbar = new CScrollbar(this, nlines-2, 1, 1, ncols-1, 0, 0, true, 'r');
    m_pHScrollbar = new CScrollbar(this, 1, ncols-2, nlines-1, 1, 0, 0, false, 'r');
}

void CTextWindow::HScroll(int n)
{
    if (m_bWrap || (m_iLongestLine <= m_pTextWin->width()))
        return;
    
    m_pHScrollbar->Scroll(n);
    refresh();
}

void CTextWindow::VScroll(int n)
{
    m_pVScrollbar->Scroll(n);
    int diff = (int)m_pVScrollbar->GetValue() - m_iCurrentLine;
    
    if (diff)
    {
        std::advance(m_CurrentLineIt, diff);
        m_iCurrentLine = (int)m_pVScrollbar->GetValue();
        refresh();
    }
}

void CTextWindow::ScrollToBottom()
{
    int h = m_FormattedText.size() - m_pTextWin->height();
    if (h < 0)
        h = 0;
    
    m_pVScrollbar->SetCurrent(h);
    
    int diff = (int)m_pVScrollbar->GetValue() - m_iCurrentLine;
    
    if (diff)
    {
        std::advance(m_CurrentLineIt, diff);
        m_iCurrentLine = (int)m_pVScrollbar->GetValue();
    }
}

bool CTextWindow::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
    switch (ch)
    {
        case KEY_LEFT:
            HScroll(-1);
            break;
        case KEY_RIGHT:
            HScroll(1);
            break;
        case KEY_UP:
            VScroll(-1);
            break;
        case KEY_DOWN:
            VScroll(1);
            break;
        case KEY_NPAGE:
            VScroll(m_pTextWin->height());
            break;
        case KEY_PPAGE:
            VScroll(-m_pTextWin->height());
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;
}

void CTextWindow::AddText(std::string text)
{
    unsigned lines = 0;
    std::string word, line;
    std::string::size_type start=0, end, len;
    
    if (!m_FormattedText.empty())
        lines = m_FormattedText.size();

    if (m_bWrap)
    {
        do
        {
            end = text.find_first_of(" \t\n", start);
            
            if (end != std::string::npos)
                word = text.substr(start, (end-start));
            else
                word = text.substr(start);
            
            if (end != std::string::npos)
            {
                start = end;
                end = text.find_first_not_of(" \t", start+1);
                if (end != std::string::npos)
                {
                    short count = ((end - start) <= (m_pTextWin->width() - start)) ? (end-start) : (m_pTextWin->width()-start);
                    word += text.substr(start, count);
                    start = end;
                }
                else
                    start++; // Start searching on next char
            }
            
            if (m_FormattedText.empty() || (m_FormattedText.back()[m_FormattedText.back().length()-1] == '\n') ||
                ((GetUnFormatLen(m_FormattedText.back())+GetUnFormatLen(word)) > m_pTextWin->width()))
            {
                m_FormattedText.push_back("");
                lines++;
            }
            
            m_FormattedText.back() += word;
            
            len = GetUnFormatLen(m_FormattedText.back());
            if (len > m_iLongestLine)
                m_iLongestLine = len;
        }
        while(end != std::string::npos);
    }
    else
    {
        std::stringstream strstrm(text);
        while(strstrm && std::getline(strstrm, line))
        {
            m_FormattedText.push_back(line);
            lines++;
            len = GetUnFormatLen(line);
            if (len > m_iLongestLine)
                m_iLongestLine = len;
        }
    }

    if (lines > m_pTextWin->height())
        m_pVScrollbar->SetMinMax(0, (lines - m_pTextWin->height()));
    
    if (m_iLongestLine > m_pTextWin->width())
        m_pHScrollbar->SetMinMax(0, (m_iLongestLine - m_pTextWin->width()));
    
    // Current line not yet initialized
    if (m_iCurrentLine == -1)
    {
        m_iCurrentLine = 0;
        m_CurrentLineIt = m_FormattedText.begin();
    }
    
    if (m_bFollow)
        ScrollToBottom();
}

void CTextWindow::LoadFile(const char *fname)
{
    std::ifstream file(fname);
    std::string line;
    
    while(file && std::getline(file, line))
        AddText(line);
}

void CTextWindow::Draw()
{
    int lines = 0; // Printed lines
    
    m_pTextWin->erase();
    
    if (!m_FormattedText.empty())
    {
        for(std::list<std::string>::iterator it=m_CurrentLineIt;
            ((it!=m_FormattedText.end()) && (lines<m_pTextWin->height())); it++)
        {
            m_pTextWin->AddStrFormat(lines, 0, it->c_str(), (int)m_pHScrollbar->GetValue(),
                                     m_pTextWin->width());
            lines++;
        }
    }

    m_pVScrollbar->Enable((HasBox() && (m_FormattedText.size() > m_pTextWin->height())));
    m_pHScrollbar->Enable(!m_bWrap && HasBox() && (m_iLongestLine > m_pTextWin->width()));        
}

// -------------------------------------
// Menu class
// -------------------------------------

chtype CMenu::m_cDefaultFocusedColors;
chtype CMenu::m_cDefaultDefocusedColors;

CMenu::CMenu(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
               char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel, true,
                                            true, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                              m_bSortItems(false), m_iCursorLine(0), m_iStartEntry(0), m_iLongestLine(0)
{
    m_pTextWin = new CWidgetWindow(this, nlines-2, ncols-2, 1, 1, 'r', false);
    m_pVScrollbar = new CScrollbar(this, nlines-2, 1, 1, ncols-1, 0, 100, true, 'r');
    m_pHScrollbar = new CScrollbar(this, 1, ncols-2, nlines-1, 1, 0, 100, false, 'r');
}

void CMenu::HScroll(int n)
{
    if (m_iLongestLine <= m_pTextWin->width())
        return;
    
    m_pHScrollbar->Scroll(n);
    refresh();
}

void CMenu::VScroll(int n)
{
    bool scroll = false;
    int newline = m_iCursorLine + n;
    
    if (newline < 0)
    {
        if ((m_iStartEntry + newline) < 0)
        {
            m_iStartEntry = m_iCursorLine = 0;
            m_pVScrollbar->SetCurrent(0);
        }
        else
            scroll = true;
    }
    else if (newline >= m_pTextWin->height())
    {
        if ((m_iStartEntry + newline) >= m_MenuItems.size())
        {
            m_iStartEntry = (m_MenuItems.size() - m_pTextWin->height());
            if (m_iStartEntry < 0)
                m_iStartEntry = 0;
            m_iCursorLine = m_pTextWin->height()-1;
            m_pVScrollbar->SetCurrent(m_MenuItems.size() - m_pTextWin->height());
        }
        else
            scroll = true;
    }
    else if (newline >= m_MenuItems.size())
        m_iCursorLine = ((m_MenuItems.size()-1)-m_iStartEntry);
    else
        m_iCursorLine = newline;

    if (scroll)
    {
        int oldstart = m_iStartEntry;
        m_pVScrollbar->Scroll(n);
        m_iStartEntry = (int)m_pVScrollbar->GetValue();
        
        if (GetCurrent() < (oldstart + newline))
            m_iCursorLine = ((oldstart + newline) - GetCurrent());
    }
    
    refresh();
}

bool CMenu::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
    if (m_bSortItems)
    {
        std::sort(m_MenuItems.begin(), m_MenuItems.end());
        m_bSortItems = false;
    }

    switch (ch)
    {
        case KEY_LEFT:
            HScroll(-1);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_RIGHT:
            HScroll(1);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_UP:
            VScroll(-1);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_DOWN:
            VScroll(1);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_ENTER:
        case '\n':
        case '\r':
        {
            PushEvent(EVENT_CALLBACK);
            break;
        }
        case KEY_NPAGE:
            VScroll(m_pTextWin->height());
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_PPAGE:
            VScroll(-m_pTextWin->height());
            PushEvent(EVENT_DATACHANGED);
            break;
        default:
            if (!isprint(ch))
                handled = false;
            else // Go to item which starts with typed character
            {
                std::vector<std::string>::iterator cur = (m_MenuItems.begin() + GetCurrent());
                
                // First try from current position
                std::vector<std::string>::iterator it = std::lower_bound(cur+1, m_MenuItems.end(), ch, LowerChar);
                if ((it == m_MenuItems.end()) || ((*it)[0] != ch))
                    it = std::lower_bound(m_MenuItems.begin(), cur, ch, LowerChar); // Failed, start from the begin
                
                if ((it != m_MenuItems.end()) && ((*it)[0] == ch))
                {
                    VScroll(std::distance(cur, it));
                    PushEvent(EVENT_DATACHANGED);
                }
            }
            break;
    }
    
    return handled;
}

void CMenu::Draw()
{
    int lines = 0; // Printed lines
    
    m_pTextWin->erase();
    
    if (!m_MenuItems.empty())
    {
        if (m_bSortItems)
        {
            std::sort(m_MenuItems.begin(), m_MenuItems.end());
            m_bSortItems = false;
        }
        
        for(int i=m_iStartEntry; ((i<m_MenuItems.size()) && (lines<m_pTextWin->height())); i++, lines++)
        {
            if (m_iCursorLine == lines)
                m_pTextWin->attron(A_REVERSE);
            
            m_pTextWin->AddStrFormat(lines, 0, m_MenuItems[i].c_str(), (int)m_pHScrollbar->GetValue(),
                                     m_pTextWin->width());
            
            if (m_iCursorLine == lines)
                m_pTextWin->attroff(A_REVERSE);
        }
    }
}

void CMenu::AddItem(std::string s)
{
    m_MenuItems.push_back(s);
    m_bSortItems = true;
    
    int h = m_MenuItems.size() - m_pTextWin->height();
    if (h < 0)
        h = 0;
    m_pVScrollbar->SetMinMax(0, h);

    unsigned len = GetUnFormatLen(s);
    if (len > m_iLongestLine)
    {
        m_iLongestLine = len;
        
        if (m_iLongestLine > m_pTextWin->width())
            m_pHScrollbar->SetMinMax(0, (m_iLongestLine - m_pTextWin->width()));
    }
    
    m_pVScrollbar->Enable((m_MenuItems.size() > m_pTextWin->height()));
    m_pHScrollbar->Enable((m_iLongestLine > m_pTextWin->width()));
}

void CMenu::Clear()
{
    m_MenuItems.clear();
    m_iCursorLine = m_iStartEntry = m_iLongestLine = 0;
    m_pHScrollbar->SetCurrent(0);
    m_pVScrollbar->SetCurrent(0);
}

// -------------------------------------
// Inputfield class
// -------------------------------------

chtype CInputField::m_cDefaultFocusedColors;
chtype CInputField::m_cDefaultDefocusedColors;

CInputField::CInputField(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel,
                         int max, chtype out) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x,
                                                              absrel, true, true, m_cDefaultFocusedColors,
                                                              m_cDefaultDefocusedColors),
                                                m_chOutChar(out), m_iMaxChars(max), m_iCursorPos(0),
                                                m_iScrollOffset(0)
{
    m_pOutputWin = new CWidgetWindow(this, nlines-2, ncols-2, 1, 1, 'r', false);
}

void CInputField::Addch(chtype ch)
{
    if ((!m_iMaxChars != -1) && (m_szText.length() >= m_iMaxChars))
        return;
    
    unsigned pos = m_iScrollOffset + m_iCursorPos;
    if (pos == m_szText.length())
        m_szText += ch;
    else
        m_szText.insert(pos, 1, ch);
    
    MoveCursor(1);
}

void CInputField::Delch(bool backspace)
{
    if (m_szText.length() < 1)
        return;
    
    unsigned pos = m_iScrollOffset + m_iCursorPos;
    
    if (backspace)
        pos--;
    
    if (pos < m_szText.length())
        m_szText.erase(pos, 1);
    
    if (backspace)
        MoveCursor(-1);
    else
    {
        if (m_iScrollOffset)
            m_iScrollOffset--;
        refresh(); // MoveCursor would call it otherwise
    }
}

void CInputField::MoveCursor(int n, bool relative)
{
    int w = (m_pOutputWin->width()-1); // -1, because we want to scroll before char is at last available position
    
    if (relative)
        m_iCursorPos += n;
    else
    {
        m_iScrollOffset = 0;
        m_iCursorPos = n;
    }
    
    if ((m_iScrollOffset + m_iCursorPos) > (int)m_szText.length())
        m_iCursorPos = (m_szText.length() - m_iScrollOffset);

    if (m_iCursorPos < 0)
    {
        m_iScrollOffset += m_iCursorPos;
        
        if (m_iScrollOffset < 0)
            m_iScrollOffset = 0;
        
        m_iCursorPos = 0;
    }
    else if (m_iCursorPos > w)
    {
        m_iScrollOffset += (m_iCursorPos - w);
        
        unsigned max = m_szText.length() - w;
        if (m_iScrollOffset > max)
            m_iScrollOffset = max;
        
        m_iCursorPos = w;
    }
    
    refresh();
}

bool CInputField::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
    switch (ch)
    {
        case KEY_LEFT:
            MoveCursor(-1);
            break;
        case KEY_RIGHT:
            MoveCursor(1);
            break;
        case KEY_HOME:
            m_iCursorPos = m_iScrollOffset = 0;
            refresh();
            break;
        case KEY_END:
            MoveCursor(m_szText.length(), false);
            break;
        case KEY_DC:
            Delch(false);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_BACKSPACE:
        case 0x07f: // Some terminals may give this instead of KEY_BACKSPACE
            Delch(true);
            PushEvent(EVENT_DATACHANGED);
            break;
        case KEY_ENTER:
        case '\n':
        case '\r':
            PushEvent(EVENT_CALLBACK);
            break;
        default:
            if (!isprint(ch))
                handled = false;
            else
            {
                Addch(ch);
                PushEvent(EVENT_DATACHANGED);
            }
            break;
    }
    
    return handled;
}

void CInputField::Draw()
{
    m_pOutputWin->erase();

    if (!m_szText.empty())
    {
        if (m_chOutChar)
        {
            std::string str(m_szText.length(), m_chOutChar);
            m_pOutputWin->AddStrFormat(0, 0, str.c_str(), m_iScrollOffset, m_pOutputWin->width());
        }
        else
            m_pOutputWin->AddStrFormat(0, 0, m_szText.c_str(), m_iScrollOffset, m_pOutputWin->width());
    }
    
    m_pOutputWin->move(0, m_iCursorPos); // Draw cursor
}

// -------------------------------------
// Progressbar class
// -------------------------------------

chtype CProgressbar::m_cDefaultFillColors;
chtype CProgressbar::m_cDefaultEmptyColors;

CProgressbar::CProgressbar(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                           int min, int max, char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y,
                                                                          begin_x, absrel, true, false,
                                                                          m_cDefaultEmptyColors,
                                                                          m_cDefaultEmptyColors),
                                                            m_fMin(min), m_fMax(max), m_fCurrent(0.0f)
{
}

void CProgressbar::Draw()
{
    float maxw = (float)width() - 2.0f; // -2 for the borders
    float fac = fabs(m_fCurrent / (m_fMax-m_fMin));
    float w = (maxw * fac);
    
    move(1, 1);
    clrtoeol();
    
    attron(m_cDefaultFillColors);
    
    for (int i=0; i<(int)w; i++)
        addch(1, i+1, ' ');
    
    attroff(m_cDefaultFillColors);
}

// -------------------------------------
// Widget Box base class
// -------------------------------------

CWidgetBox::CWidgetBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                       const char *info, chtype fcolor,
                       chtype dfcolor) : CWidgetWindow(owner, maxlines, ncols, begin_y, begin_x, true,
                                                       fcolor, dfcolor), m_bFinished(false),
                                         m_bCanceled(false), m_pWidgetManager(owner)
{
    m_pLabel = new CTextLabel(this, maxlines-4, ncols-4, 2, 2, 'r');
    m_pLabel->AddText(info);
}

bool CWidgetBox::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    if (ch == CTRL('[')) // Escape
    {
        m_bFinished = m_bCanceled = true;
        return true;
    }
    
    return false;
}

void CWidgetBox::Fit(int nlines)
{
    resize(nlines, maxx());
    mvwin(((MaxY() - maxy())/2), ((MaxX() - maxx())/2));
    
    // Refresh main screen, this is required after a window has been resized or moved
    ::erase();
    m_pWidgetManager->Refresh();
}

// -------------------------------------
// Message Box class
// -------------------------------------

chtype CMessageBox::m_cDefaultFocusedColors;
chtype CMessageBox::m_cDefaultDefocusedColors;

CMessageBox::CMessageBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                         const char *text) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x,
                                                        text, m_cDefaultFocusedColors,
                                                        m_cDefaultDefocusedColors)
{
    m_pOKButton = new CButton(this, 1, 10, (m_pLabel->rely()+m_pLabel->maxy()+2), (ncols-10)/2, "OK", 'r');
    Fit(m_pOKButton->rely()+m_pOKButton->maxy()+2);
}

bool CMessageBox::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pOKButton)
    {
        m_bFinished = true;
        return true;
    }
    
    return false;
}

// -------------------------------------
// Warning Box class
// -------------------------------------

chtype CWarningBox::m_cDefaultFocusedColors;
chtype CWarningBox::m_cDefaultDefocusedColors;

CWarningBox::CWarningBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                         const char *text) : CMessageBox(owner, maxlines, ncols, begin_y, begin_x, text)
{
    SetTitle("Warning");
    SetColors(m_cDefaultFocusedColors, m_cDefaultDefocusedColors);
    m_pLabel->SetColors(m_cDefaultFocusedColors, m_cDefaultDefocusedColors);
    m_pOKButton->SetColors(m_cDefaultFocusedColors, m_cDefaultDefocusedColors);
    refresh();
}

// -------------------------------------
// Yes-No Box class
// -------------------------------------

chtype CYesNoBox::m_cDefaultFocusedColors;
chtype CYesNoBox::m_cDefaultDefocusedColors;

CYesNoBox::CYesNoBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                     const char *text) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x,
                                                    text, m_cDefaultFocusedColors,
                                                    m_cDefaultDefocusedColors)
{
    int x = (maxx()-((2*10)+2))/2; // Center 2 buttons of 10 length and one 2 sized space
    int y = (m_pLabel->rely()+m_pLabel->maxy()+2);
    m_pNoButton = new CButton(this, 1, 10, y, x, "No", 'r');
    m_pYesButton = new CButton(this, 1, 10, y, (x+m_pNoButton->maxx()+2), "Yes", 'r');
    
    Fit(y+m_pNoButton->maxy()+2);
}

bool CYesNoBox::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pYesButton)
    {
        m_bFinished = true;
        return true;
    }
    else if (p == m_pNoButton)
    {
        m_bFinished = m_bCanceled = true;
        return true;
    }
    
    return false;
}

// -------------------------------------
// Choice Box class
// -------------------------------------

chtype CChoiceBox::m_cDefaultFocusedColors;
chtype CChoiceBox::m_cDefaultDefocusedColors;

CChoiceBox::CChoiceBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                       const char *text, const char *but1, const char *but2,
                       const char *but3) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x, text,
                                                      m_cDefaultFocusedColors, m_cDefaultDefocusedColors)
{
    // Button 3 is optional
    int bcount = (but3) ? 3 : 2;
    
    // Find out longest text
    int w;
    if (but3)
        w = Max(Max(strlen(but1), strlen(but2)), strlen(but3));
    else
        w = Max(strlen(but1), strlen(but2));
    
    w += 4; // Buttons use 4 additional tokens for focusing
    
    int x = (maxx()-((bcount*w)+bcount-1))/2; // Center bcount buttons, with bcount-1 2 sized spaces.
    int y = (m_pLabel->rely()+m_pLabel->maxy()+2);

    m_pButtons[0] = new CButton(this, 1, w, y, x, but1, 'r');
    
    x += (w + 2);
    m_pButtons[1] = new CButton(this, 1, w, y, x, but2, 'r');
    
    if (but3)
    {
        x += (w + 2);
        m_pButtons[2] = new CButton(this, 1, w, y, x, but3, 'r');
    }
    
    Fit(y+m_pButtons[0]->maxy()+2);
}

bool CChoiceBox::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pButtons[0])
    {
        m_iSelectedButton = 0;
        m_bFinished = true;
    }
    else if (p == m_pButtons[1])
    {
        m_iSelectedButton = 1;
        m_bFinished = true;
    }
    else if (p == m_pButtons[2])
    {
        m_iSelectedButton = 2;
        m_bFinished = true;
    }

    return false;
}

int CChoiceBox::Run()
{
    while (m_pWidgetManager->Run() && !m_bFinished)
        ;
    
    return (m_bCanceled) ? -1 : m_iSelectedButton;
}

// -------------------------------------
// Input dialog class
// -------------------------------------

chtype CInputDialog::m_cDefaultFocusedColors;
chtype CInputDialog::m_cDefaultDefocusedColors;

CInputDialog::CInputDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                           const char *info, int max,
                           bool sec) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x, info,
                                                  m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                                       m_bSecure(sec)
{
    char out = (sec) ? '*' : 0;
    int y = (m_pLabel->rely()+m_pLabel->maxy()+1);
    m_pTextField = new CInputField(this, 3, ncols-4, y, 2, 'r', max, out);
    
    int x = ((ncols + 2) - (2 * 20 + 2)) / 2;
    y += (m_pTextField->maxy() + 2);
    m_pOKButton = new CButton(this, 1, 20, y, x, "OK", 'r');
    
    m_pCancelButton = new CButton(this, 1, 20, y, x+22, "Cancel", 'r');
    
    ActivateChild(m_pTextField);
    
    Fit(m_pOKButton->rely()+m_pOKButton->maxy()+2);
}

bool CInputDialog::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pTextField)
    {
        if (type == EVENT_CALLBACK)
        {
            m_bFinished = true;
            m_bCanceled = false;
            return true;
        }
    }
    else if (p == m_pOKButton)
    {
        m_bFinished = true;
        m_bCanceled = false;
        return true;
    }
    else if (p == m_pCancelButton)
    {
        m_bFinished = m_bCanceled = true;
        return true;
    }
    
    return false;
}

const std::string &CInputDialog::Run()
{
    while (m_pWidgetManager->Run() && !m_bFinished)
        ;
    
    std::string dummy;
    return (m_bCanceled) ? dummy : m_pTextField->GetText();
}

// -------------------------------------
// File dialog class
// -------------------------------------

chtype CFileDialog::m_cDefaultFocusedColors;
chtype CFileDialog::m_cDefaultDefocusedColors;

CFileDialog::CFileDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *s,
                         const char *info, bool w) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x, info,
                                                                m_cDefaultFocusedColors,
                                                                m_cDefaultDefocusedColors),
                                                     m_szStartDir(s), m_szSelectedDir(s), m_szInfo(info),
                                                     m_bRequireWAccess(w)
{
    m_pFileMenu = new CMenu(this, maxlines-10, ncols-4, (m_pLabel->rely()+m_pLabel->maxy()+2), 2, 'r');
    
    m_pFileField = new CInputField(this, 3, ncols-4, (m_pFileMenu->rely()+m_pFileMenu->maxy()+1), 2, 'r');
    m_pFileField->SetText(m_szStartDir);
    
    const int startx = ((ncols + 2) - (2 * 20 + 2)) / 2;
    const int starty = (m_pFileField->rely()+m_pFileField->maxy()+1);
    m_pOpenButton = new CButton(this, 1, 20, starty, startx, "Open directory", 'r');
    
    m_pCancelButton = new CButton(this, 1, 20, starty, startx+22, "Cancel", 'r');
    
    ActivateChild(m_pFileMenu);
    
    Fit(m_pOpenButton->rely()+m_pOpenButton->maxy()+2);
    
    OpenDir();
}

void CFileDialog::OpenDir(std::string newdir)
{
    if (newdir.empty())
    {
        newdir = m_szSelectedDir;

        if (!m_pFileMenu->Empty())
            newdir += '/' + m_pFileMenu->GetCurrentItemName();
    }
    
    if (chdir(newdir.c_str()))
    {
        /* UNDONE
        WarningBox("%s\n%s\n%s", GetTranslation("Could not change to directory"), newdir.c_str(), strerror(errno));
        return false;*/
        return;
    }
    
    char tmp[1024];
    if (getcwd(tmp, sizeof(tmp))) m_szSelectedDir = tmp;
    else { /*WarningBox("Could not read current directory"); return false;UNDONE*/ return; }
    
    struct dirent *dirstruct;
    struct stat filestat;
    DIR *dp = opendir(m_szSelectedDir.c_str());
    bool isrootdir = (m_szSelectedDir == "/");

    if (!dp)
        return; // UNDONE

    m_pFileMenu->Clear();
    
    while ((dirstruct = readdir (dp)) != 0)
    {
        if (lstat(dirstruct->d_name, &filestat) != 0)
            continue;
        
        if (!S_ISDIR(filestat.st_mode))
            continue; // Has to be a directory...

        if (!strcmp(dirstruct->d_name, "."))
            continue;

        if (isrootdir && !strcmp(dirstruct->d_name, ".."))
            continue;
        
        m_pFileMenu->AddItem(dirstruct->d_name);
    }

    closedir (dp);
    
    m_pFileMenu->refresh();
    
    // Update AFTER file menu, since it may sort items in Draw()
    UpdateDirField();
}

bool CFileDialog::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pFileMenu)
    {
        if (type == EVENT_CALLBACK)
        {
            OpenDir();
            return true;
        }
    }
    else if (p == m_pFileField)
    {
        if (type == EVENT_CALLBACK)
        {
            OpenDir(m_pFileField->GetText());
            return true;
        }
    }
    else if (p == m_pOpenButton)
    {
        m_bFinished = true;
        return true;
    }
    else if (p == m_pCancelButton)
    {
        m_szSelectedDir.clear();
        m_bFinished = m_bCanceled = true;
        return true;
    }
    
    return false;
}

bool CFileDialog::HandleKey(chtype ch)
{
    if (CWidgetBox::HandleKey(ch))
        return true;
    
    if (ch == KEY_F(2))
    {
        std::string newdir = InputDialog("Enter name of new directory", NULL, 1024);
        
        if (newdir.empty())
            return true;
        
        if (mkdir(newdir.c_str(), (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) != 0)
        {
            WarningBox("%s\n%.75s\n%.75s", "Could not create directory", newdir.c_str(), strerror(errno));
            return true;
        }

        OpenDir(newdir);
    }
    
    return false;
}

const std::string &CFileDialog::Run()
{
    while (m_pWidgetManager->Run() && !m_bFinished)
        ;
    
    std::string dummy;
    return (m_bCanceled) ? dummy : m_szSelectedDir;
}
