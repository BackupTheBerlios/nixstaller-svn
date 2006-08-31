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
        return ((*m_FocusedChild)->HandleKey(ch) || (m_pBoundKeyWidget && m_pBoundKeyWidget->HandleKey(ch)));
    
    return false;
}

bool CWidgetHandler::SetNextWidget(bool rec)
{
    if (m_ChildList.empty())
    {
        m_FocusedChild = m_ChildList.end();
        return !Focused();
    }
        
    std::list<CWidgetWindow *>::iterator it = ((Focused()) ? m_FocusedChild : m_ChildList.begin()), prev;
    prev = it;
    
    while (it != m_ChildList.end())
    {
        if ((*it)->CanFocus() && (*it)->Enabled() && !(*it)->m_bDeleteMe && (!rec || (*it)->SetNextWidget()))
        {
            if (prev != it)
            {
                (*prev)->LeaveFocus();
                (*it)->Focus();
                m_FocusedChild = it;
            }
            return true;
        }
        
        it++;
    }
    
    return false;
}

bool CWidgetHandler::SetPrevWidget(bool rec)
{
    if (m_ChildList.empty())
    {
        m_FocusedChild = m_ChildList.end();
        return !Focused();
    }
        
    std::list<CWidgetWindow *>::iterator it = ((Focused()) ? m_FocusedChild : m_ChildList.end()), prev;
    
    if (it == m_ChildList.end())
        it--;
    
    prev = it;
    
    while (true)
    {
        if ((*it)->CanFocus() && (*it)->Enabled() && !(*it)->m_bDeleteMe && (!rec || (*it)->SetPrevWidget()))
        {
            if (prev != it)
            {
                (*prev)->LeaveFocus();
                (*it)->Focus();
                m_FocusedChild = it;
            }
            return true;
        }
        
        if (it == m_ChildList.begin())
            break;
        
        it--;
    }
    
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
    {
        if (!SetPrevWidget())
            SetNextWidget();
    }
    
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
        {
            (*m_FocusedChild)->LeaveFocus();
        }
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
        {
            if (!m_pOwner->SetPrevWidget(false))
                m_pOwner->SetNextWidget(false);
        }
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

    CInputField::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_RED) | A_BOLD;
    CInputField::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_RED) | A_BOLD;
    
    CProgressbar::m_cDefaultFillColors = ' ' | CWidgetWindow::GetColorPair(COLOR_BLUE, COLOR_YELLOW) | A_BOLD;
    CProgressbar::m_cDefaultEmptyColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
    
    CCheckbox::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CCheckbox::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

    CRadioButton::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CRadioButton::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;

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
    
    CMenuDialog::m_cDefaultFocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_YELLOW, COLOR_BLUE) | A_BOLD;
    CMenuDialog::m_cDefaultDefocusedColors = ' ' | CWidgetWindow::GetColorPair(COLOR_WHITE, COLOR_BLUE) | A_BOLD;
}

void CWidgetManager::Refresh()
{
    ::refresh();

    for (std::list<CWidgetWindow *>::iterator it=m_ChildList.begin(); it!=m_ChildList.end(); it++)
    {
        if ((*it)->Enabled() && !(*it)->m_bDeleteMe)
            (*it)->refresh();
    }
}

void CWidgetManager::RemoveChild(CWidgetWindow *p)
{
    p->erase();
    
    p->m_bDeleteMe = true;

    if (*m_FocusedChild == p)
    {
        if (!SetPrevWidget(false))
            SetNextWidget(false);
    }
    
    ::erase();
    Refresh();
}

void CWidgetManager::ActivateChild(CWidgetWindow *p)
{
    if (m_ChildList.back() != p)
    { 
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
    CWidgetHandler::ActivateChild(p);
}

bool CWidgetManager::Run()
{
    if (m_bQuit)
        return false;
    
    // Check for widgets that should be removed
    bool done = false;
    while(!done)
    {
        std::list<CWidgetWindow *>::iterator it = m_ChildList.begin();
        for (;it!=m_ChildList.end(); it++)
        {
            if ((*it)->m_bDeleteMe)
            {
                if (m_FocusedChild == it)
                {
                    if (!SetPrevWidget(false))
                        SetNextWidget(false);
                }
             
                bool changed = (m_FocusedChild != it);
                m_ChildList.erase(it);
    
                if (!changed)
                {
                    // Have to do this after the child was removed from the list, otherwise it would be invalid
                    m_FocusedChild = m_ChildList.end();
                }
             
                delete *it;
                break; // Iter was removed, start over
            }
        }
        done = (it == m_ChildList.end());
    }
    
    if (m_FocusedChild != m_ChildList.end())
    {
        chtype ch = (*m_FocusedChild)->getch();
        if (ch != ERR)
        {
            if (!(*m_FocusedChild)->HandleKey(ch))
            {
                if (!m_pBoundKeyWidget || !m_pBoundKeyWidget->HandleKey(ch))
                {
                    if ((ch == 9) || (ch == CTRL('n')))
                        SetNextChildWidget();
                    else if ((ch == KEY_BTAB) || (ch == CTRL('p')))
                        SetPrevChildWidget();
                    else if (ch == CTRL('[')) // Escape pressed
                    {
                        // Set this so that later calls will also return false. This is required incase this function
                        // wasn't called from the main app.
                        m_bQuit = true;
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

bool CWidgetManager::SetNextChildWidget()
{
    if (m_ChildList.empty())
    {
        m_FocusedChild = m_ChildList.end();
        return false;
    }
    
    std::list<CWidgetWindow *>::iterator prev = (*m_FocusedChild)->m_FocusedChild, cur;
    
    if (prev == (*m_FocusedChild)->m_ChildList.end())
        prev = (*m_FocusedChild)->m_ChildList.begin();
    
    cur = prev;

    do
    {
        if (cur == (*m_FocusedChild)->m_ChildList.end())
            cur = (*m_FocusedChild)->m_ChildList.begin();
        
        if ((*cur)->CanFocus() && (*cur)->Enabled() && !(*cur)->m_bDeleteMe && (*cur)->SetNextWidget())
        {
            if (cur != prev)
            {
                (*prev)->LeaveFocus();
                (*m_FocusedChild)->m_FocusedChild = cur;
                (*cur)->Focus();
            }
            return true;
        }
        cur++;
    }
    while (cur != prev);
    
    return false;
}

bool CWidgetManager::SetPrevChildWidget()
{
    if (m_ChildList.empty())
    {
        m_FocusedChild = m_ChildList.end();
        return false;
    }
    
    std::list<CWidgetWindow *>::iterator prev = (*m_FocusedChild)->m_FocusedChild, cur;
    
    if (prev == (*m_FocusedChild)->m_ChildList.end())
        prev = (*m_FocusedChild)->m_ChildList.begin();
    
    cur = prev;

    do
    {
        if ((*cur)->CanFocus() && (*cur)->Enabled() && !(*cur)->m_bDeleteMe && (*cur)->SetPrevWidget())
        {
            if (cur != prev)
            {
                (*prev)->LeaveFocus();
                (*m_FocusedChild)->m_FocusedChild = cur;
                (*cur)->Focus();
            }
            return true;
        }
        
        if (cur == (*m_FocusedChild)->m_ChildList.begin())
            cur = (*m_FocusedChild)->m_ChildList.end();

        cur--;
    }
    while (cur != prev);
    
    return false;
}

// -------------------------------------
// Formatted text class
// -------------------------------------

CFormattedText::CFormattedText(CWidgetWindow *w, const std::string &str, bool wrap,
                               unsigned maxh) : m_pWindow(w), m_uWidth(w->width()), m_uMaxHeight(maxh), m_bWrap(wrap)
{
    if (!str.empty())
        AddText(str);
}

std::map<unsigned, CFormattedText::color_entry_s *>::iterator
        CFormattedText::GetNextColorTag(std::map<unsigned, CFormattedText::color_entry_s *>::iterator cur, unsigned curpos)
{
    std::map<unsigned, color_entry_s *>::iterator ret = m_ColorTags.end();
    if (cur != m_ColorTags.begin())
        cur++; // Start searching on next iter
    
    for (std::map<unsigned, color_entry_s *>::iterator it=cur; it!=m_ColorTags.end(); it++)
    {
        if (!it->second)
            continue;
        
        if ((it->first > curpos) || (it->second->count >= (curpos - it->first)+1)) // In reach
        {
            ret = it;
            break;
        }
    }
    
    return ret;
}

std::map<unsigned, int>::iterator CFormattedText::GetNextRevTag(std::map<unsigned, int>::iterator cur, unsigned curpos)
{
    std::map<unsigned, int>::iterator ret = m_ReversedTags.end();
    if (cur != m_ReversedTags.begin())
        cur++; // Start searching on next iter
    
    for (std::map<unsigned, int>::iterator it=cur; it!=m_ReversedTags.end(); it++)
    {
        if (!it->second)
            continue;
        
        if ((it->first > curpos) || (it->second >= (curpos - it->first)+1)) // In reach
        {
            ret = it;
            break;
        }
    }
    
    return ret;
}

void CFormattedText::AddText(const std::string &str)
{
    if (m_Lines.size() > m_uMaxHeight)
        return;
    
    unsigned offset = m_szRawText.length();
    
    std::string newtext;
    
    unsigned strstart = 0, strend, length = str.length();
    unsigned addedchars = offset;
    unsigned startcolor = 0, startrev = 0;
    int fgcolor = -1, bgcolor = -1;

    while (strstart < length)
    {
        if (str[strstart] == '<')
        {
            if (!str.compare(strstart+1, 4, "col=")) // Color tag
            {
                startcolor = addedchars;
                strstart += 5; // length of <col= == 5
                
                // Get foreground color
                strend = str.find(":", strstart);
                fgcolor = atoi(str.substr(strstart, (strend-strstart)+1).c_str());
                
                // Get background color
                strstart = strend+1;
                strend = str.find(">", strstart);
                bgcolor = atoi(str.substr(strstart, (strend-strstart)+1).c_str());
                
                strstart = strend+1;
                continue;
            }
            else if (!str.compare(strstart+1, 5, "/col>")) // End color tag
            {
                if ((fgcolor != -1) && (bgcolor != -1))
                    m_ColorTags[startcolor] = new color_entry_s(addedchars-startcolor, fgcolor, bgcolor);
                strstart += 6;
                fgcolor = bgcolor = -1;
                continue;
            }
            else if (!str.compare(strstart+1, 4, "rev>")) // Reverse tag
            {
                startrev = addedchars;
                strstart += 5; // "<rev>";
                continue;
            }
            else if (!str.compare(strstart+1, 5, "/rev>")) // End reverse tag
            {
                m_ReversedTags[startrev] = addedchars-startrev;
                strstart += 6; // "</rev>";
                continue;
            }
            else if (!str.compare(strstart+1, 2, "C>"))
            {
                m_CenteredIndexes.insert(addedchars);
                strstart += 3;
                continue;
            }
        }
        
        m_szRawText += str[strstart];
        addedchars++;
        strstart++;
    }
    
    // Now process tagless text
    strstart = offset;
    length = m_szRawText.length();
    
    if (m_Lines.empty())
    {
        m_Lines.push_back(new line_entry_s);
        m_uCurrentLine = 0;
    }
    
    if (m_bWrap)
    {
        do
        {
            strend = strstart + (m_uWidth-1);
            
            if (strend < strstart) // Overflow
                strend = std::numeric_limits<unsigned>::max()-1;
            
            bool add = false;
            if (!m_Lines[m_uCurrentLine]->text.empty())
                strend -= ((strend-strstart) - (m_Lines[m_uCurrentLine]->text.length()-1));
            
            if (strend >= length)
                strend = length - 1;
            
            unsigned newline = m_szRawText.substr(strstart, (strend-strstart)+1).find("\n");
            if (newline != std::string::npos)
            {
                strend = strstart + newline;
                add = true; // Found a newline, so create a new line entry at the end of the loop
            }
            
            if ((strend + 1) != length)
            {
                unsigned begwind = strend;
                while ((begwind >= strstart) && !isspace(m_szRawText[begwind]))
                    begwind--;
                
                if ((begwind >= strstart) && (begwind != strend))
                {
                    unsigned endwind = m_szRawText.find_first_of(" \t\n", begwind+1);
                    if (endwind == std::string::npos)
                        endwind = length; // NOT -1, because normally endwind would be point to a whitespace after the word
                    
                    if (((endwind - begwind)-1 <= m_uWidth) && (endwind-1 != strend)) 
                        strend = begwind;
                }
            }
            
            if ((strend - strstart) > 0)
            {
                bool toolong = ((m_Lines[m_uCurrentLine]->text.length() + (strend-strstart)+1) > m_uWidth);
                
                if (((strend+1) < length) && isspace(m_szRawText[strend+1]))
                    strend++; // Don't add leading whitespace to a new line
    
                if (!toolong) // If it's not too long add to current line
                    m_Lines[m_uCurrentLine]->text += m_szRawText.substr(strstart, (strend - strstart) + 1);
                
                if (((strend+1)>=length) || add || toolong)//(m_Lines[m_uCurrentLine]->text.length() >= m_uWidth))
                {
                    if (m_Lines.size() >= m_uMaxHeight)
                        break;
    
                    m_Lines.push_back(new line_entry_s);
                    m_uCurrentLine++;
                    
                    if (toolong) // New line was too long, add to new one
                        m_Lines[m_uCurrentLine]->text = m_szRawText.substr(strstart, (strend - strstart) + 1);
                }
                
                unsigned newlen = m_Lines[m_uCurrentLine]->text.length();
                if (newlen > m_uLongestLine)
                    m_uLongestLine = newlen;
            }
            
            strstart = strend + 1;
        }
        while (strstart < length);
    }
    else
    {
        do
        {
            strend = m_szRawText.find("\n", strstart);
            
            if (strend != std::string::npos)
                m_Lines[m_uCurrentLine]->text = m_szRawText.substr(strstart, (strend-strstart)+1);
            else
                m_Lines[m_uCurrentLine]->text = m_szRawText.substr(strstart);
            
            unsigned newlen = m_Lines[m_uCurrentLine]->text.length();
            if (newlen > m_uLongestLine)
                m_uLongestLine = newlen;

            m_uCurrentLine++;
            m_Lines.push_back(new line_entry_s);
            
            strstart = strend + 1;
        }
        while ((strend != std::string::npos) && ((strend+1) < length) && (m_uCurrentLine < m_uMaxHeight));
    }
    
    bool m_bSort = true;
    
    if (m_bSort)
    {
        std::vector<line_entry_s *> origvec = m_Lines;
        std::sort(m_Lines.begin(), m_Lines.end(), LessThan);

        // Restore all tag positions
        unsigned chars = 0;
        for (unsigned u=0; u<m_Lines.size(); u++)
        {
            if (origvec[u] != m_Lines[u])
            {
                for (std::map<unsigned, color_entry_s *>::iterator it=m_ColorTags.begin(); it!=m_ColorTags.end(); it++)
                {
                    if ((it->first >= chars) && (it->first < (chars+origvec[u]->text.length())))
                    {
                        //std::vector<line_entry_s *>::iterator newit = std::lower_bound(m_Lines.begin(), m_Lines.end(), *(origvec.begin()+u), LessThan);
                        std::vector<line_entry_s *>::iterator newit = std::find(m_Lines.begin(), m_Lines.end(), *(origvec.begin()+u));
                        debugline("color tag moved from line %u to %u('%s')\n", u, std::distance(m_Lines.begin(), newit), origvec[u]->text.c_str());
                        AddColorTag(std::distance(m_Lines.begin(), newit), (it->first-chars), it->second->count, it->second->fgcolor, it->second->bgcolor);
                        DelColorTag(u, (it->first-chars));
                    }
                }
            }
            chars += m_Lines[u]->text.length();
        }
    }
}

void CFormattedText::Print(unsigned startline, unsigned startw, unsigned endline, unsigned endw)
{
    if (m_Lines.empty())
        return;
    
    unsigned chars = 0, index, y=0;
    
    if ((endline + (startline+1)) < endline)
        endline = std::numeric_limits<unsigned>::max(); // Overflow
    else
        endline += (startline+1);
    
    if (endline > m_Lines.size())
        endline = m_Lines.size();

    // Add characters we don't print
    if (startline > 0)
    {
        for (unsigned u=0; u<startline; u++)
            chars += m_Lines[u]->text.length();
    }
    
    std::map<unsigned, color_entry_s *>::iterator colorit = GetNextColorTag(m_ColorTags.begin(), 0);
    std::map<unsigned, int>::iterator revit = GetNextRevTag(m_ReversedTags.begin(), 0);
    
    bool initcolortag = (colorit != m_ColorTags.end());
    bool initrevtag = (revit != m_ReversedTags.end());
    
    /*
    std::vector<std::string> Sorted;
    for (unsigned u=startline; u<endline; u++)
    {
        Sorted.push_back(m_Lines[u]->text);
    }
    
    std::sort(Sorted.begin(), Sorted.end(), LessThan);
    std::map<unsigned, unsigned> sortedpos;
    for (unsigned u=startline; u<endline; u++)
    {
        for (unsigned u2=0; u2<Sorted.size(); u2++)
        {
            if (Sorted[u2] == m_Lines[u]->text)
            {
                sortedpos[u] = u2;
                //debugline("%u --> %u : %s\n", u, u2, m_Lines[u]->text.c_str());
                break;
            }
        }
    }
    
    debugline("Sorted:\n");
    for (unsigned u2=0; u2<Sorted.size(); u2++)
    debugline("%d. %s\n", u2, Sorted[u2].c_str());*/
    
    for (unsigned u=startline; u<endline; u++, y++)
    {
        unsigned len = m_Lines[u]->text.length(), spaces = 0;
        
        index = startw;
        
        if (m_bWrap) // Only enable centering while wrapping for now
        {
            for (std::set<unsigned>::iterator it=m_CenteredIndexes.begin(); it!=m_CenteredIndexes.end(); it++)
            {
                //unsigned centerw = (m_bWrap) ? m_uWidth : Max(m_uWidth, m_uLongestLine);
                unsigned centerw = m_uWidth;
                if ((*it >= chars) && (*it <= (chars+centerw)) && (*it <= (chars+len)))
                {
                    std::string tmp = m_Lines[u]->text;
                    EatWhite(tmp);
                    
                    spaces = (centerw-tmp.length())/2;
                    
                    if (spaces <= startw)
                        spaces = 0; // Can't center, starting width is too high
                    else
                        spaces -= startw;
                    
                    break;
                }
            }
        }
        
        unsigned count = (len > endw) ? endw : len;
        unsigned x = 0;
        
        if (((index+1) + count) < count)
            count = std::numeric_limits<unsigned>::max(); // Overflow
        else
            count += (index+1);
    
        if (count > len)
            count = len;
        
        chars += index;
        
        while (index < count)
        {
            // Line may containing trailing whitespace or newline which should not be printed.
            bool canprint = (!m_bWrap || (index <= m_uWidth)) && (((index+1)!=len) || !isspace(m_Lines[u]->text[index]));
            
            if ((colorit != m_ColorTags.end()) && (colorit->first <= chars) && (colorit->second->count < (chars-colorit->first)+1))
            {
                m_pWindow->attroff(CWidgetWindow::GetColorPair(colorit->second->fgcolor,
                                   colorit->second->bgcolor));
                colorit = GetNextColorTag(colorit, chars);
                initcolortag = (colorit != m_ColorTags.end());
            }
            
            if ((revit != m_ReversedTags.end()) && (revit->first <= chars) && (revit->second < (chars-revit->first)+1))
            {
                m_pWindow->attroff(A_REVERSE);
                revit = GetNextRevTag(revit, chars);
                initrevtag = (revit != m_ReversedTags.end());
            }

            if (initcolortag && (colorit != m_ColorTags.end()) && (colorit->first <= chars))
            {
                m_pWindow->attron(CWidgetWindow::GetColorPair(colorit->second->fgcolor,
                                  colorit->second->bgcolor));
                initcolortag = false;
            }
            
            if (initrevtag && (revit != m_ReversedTags.end()) && (revit->first <= chars))
            {
                m_pWindow->attron(A_REVERSE);
                initrevtag = false;
            }

            if (canprint)
                m_pWindow->addch(y, spaces+x, m_Lines[u]->text[index]);
                //m_pWindow->addch(sortedpos[u], spaces+x, m_Lines[u]->text[index]);
            
            index++;
            chars++;
            x++;
        }
        chars += (len-count);
    }
    
    // Disable enabled tags
    if (colorit != m_ColorTags.end())
        m_pWindow->attroff(CWidgetWindow::GetColorPair(colorit->second->fgcolor,
                           colorit->second->bgcolor));
    
    if (revit != m_ReversedTags.end())
        m_pWindow->attroff(A_REVERSE);
}

unsigned CFormattedText::GetLines()
{
    unsigned ret = m_Lines.size();
    if (ret && m_Lines[ret-1]->text.empty()) // Last line may be empty
        ret--;
    return ret;
}

void CFormattedText::Clear()
{
    for (std::map<unsigned, color_entry_s *>::iterator it=m_ColorTags.begin(); it!=m_ColorTags.end(); it++)
        delete it->second;
    
    for (std::vector<line_entry_s *>::iterator it=m_Lines.begin(); it!=m_Lines.end(); it++)
        delete *it;
    
    m_ColorTags.clear();
    m_Lines.clear();
    m_szRawText.clear();
    m_CenteredIndexes.clear();
    m_uCurrentLine = m_uLongestLine = 0;
}

void CFormattedText::AddCenterTag(unsigned line)
{
    unsigned chars = 0;
    for (unsigned u=0; u<m_Lines.size(); u++)
    {
        if (u < line)
            chars += m_Lines[u]->text.length();
        else
            break;
    }

    m_CenteredIndexes.insert(chars+1);
}

void CFormattedText::AddRevTag(unsigned line, unsigned pos, unsigned c)
{
    unsigned chars = 0;
    for (unsigned u=0; u<m_Lines.size(); u++)
    {
        if (u < line)
            chars += m_Lines[u]->text.length();
        else
        {
            chars += pos;
            break;
        }
    }

    m_ReversedTags[chars] = c;
}

void CFormattedText::AddColorTag(unsigned line, unsigned pos, unsigned c, int fg, int bg)
{
    unsigned chars = 0;
    for (unsigned u=0; u<m_Lines.size(); u++)
    {
        if (u < line)
            chars += m_Lines[u]->text.length();
        else
        {
            chars += pos;
            break;
        }
    }

    m_ColorTags[chars] = new color_entry_s(c, fg, bg);
}

void CFormattedText::DelCenterTag(unsigned line)
{
    unsigned chars = 0;
    for (unsigned u=0; u<m_Lines.size(); u++)
    {
        if (u < line)
            chars += m_Lines[u]->text.length();
        else
            break;
    }

    m_CenteredIndexes.erase(chars+1);
}

void CFormattedText::DelRevTag(unsigned line, unsigned pos)
{
    unsigned chars = 0;
    for (unsigned u=0; u<m_Lines.size(); u++)
    {
        if (u < line)
            chars += m_Lines[u]->text.length();
        else
        {
            chars += pos;
            break;
        }
    }

    m_ReversedTags.erase(chars);
}

void CFormattedText::DelColorTag(unsigned line, unsigned pos)
{
    unsigned chars = 0;
    for (unsigned u=0; u<m_Lines.size(); u++)
    {
        if (u < line)
            chars += m_Lines[u]->text.length();
        else
        {
            chars += pos;
            break;
        }
    }

    m_ColorTags.erase(chars);
}

// -------------------------------------
// Widget window class
// -------------------------------------

CWidgetWindow::ColorMapType CWidgetWindow::m_ColorPairs;
int CWidgetWindow::m_iCurColorPair = 0;
chtype CWidgetWindow::m_cDefaultFocusedColors;
chtype CWidgetWindow::m_cDefaultDefocusedColors;
int CWidgetWindow::m_iCursorY = -1;
int CWidgetWindow::m_iCursorX = -1;

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
    if (!Enabled() || m_bDeleteMe)
        return 0;
    
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
    
    ApplyCursorPos();
    ::refresh();
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
    m_szTitle = "<C>";
    m_szTitle += text;
}

bool CButton::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    if (ENTER(ch))
    {
        PushEvent(EVENT_CALLBACK);
        return true;
    }
    
    return false;
}

void CButton::Draw()
{
    erase();
    
    AddStrFormat(0, 0, m_szTitle);
    
    if (Focused())
    {
        addch(0, 0, '<');
        addch(0, maxx(), '>');
    }
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
                       char absrel) : CWidgetWindow(owner, 1, ncols, begin_y, begin_x, absrel, false,
                                                   false, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                                      m_iMaxHeight(nlines), m_uCurLines(1), m_FMText(this, "", true, nlines)
{
}

void CTextLabel::AddText(std::string text)
{
    unsigned curlines = m_uCurLines;
    m_FMText.AddText(text);
    
    if (curlines != m_FMText.GetLines())
    {
        m_uCurLines = m_FMText.GetLines();
        resize(m_uCurLines, width());
    }
}


void CTextLabel::Draw()
{
    erase();
    m_FMText.Print();
}

int CTextLabel::CalcHeight(int ncols, const std::string &text)
{
#if 1
    unsigned lines = 0, wordlen = 0, linelen = 0;
    std::string::size_type start=0, end;
    
    do
    {
        end = text.find_first_of(" \t\n", start);
        
        if ((end != std::string::npos) && (text[end] == '\n'))
            end++; // Include \n in word

        if (GetUnFormatLen(text.substr(start, end-start)) >= ncols)
        {
            wordlen = ncols;
            start += ncols;
            end = start; // HACK: So that loop won't be broken
        }
        else if (end != std::string::npos)
        {
            wordlen = GetUnFormatLen(text.substr(start, (end-start)));
            start = end;
            
            end = text.find_first_not_of(" \t", start);
            if (end != std::string::npos)
            {
                wordlen += (end - start);
                start = end;
            }
            else
                start++; // Start searching on next char
        }
        else
            wordlen = GetUnFormatLen(text.substr(start));

        if (!lines || (linelen+wordlen) >= ncols)
        {
            lines++;
            linelen = 0;
        }
        else
            linelen += wordlen;
    }
    while (end != std::string::npos);
    
    return lines + (linelen > 0);
#else
    unsigned lines = 1, start = 0, end = ncols, length = text.length();
        
    do
    {
        end = start + width()-1;
        
        if (end >= length)
            end = length - 1;
        
        unsigned newline = text.substr(start, (end-start)+1).find("\n");
        if (newline != std::string::npos)
            end = newline+1;
        
        if ((end + 1) != length)
        {
            while ((end >= start) && !isspace(text[end]))
                end--;
        }
        
        if ((end - start) > 0)
            lines++;
        
        start = end + 1;
    }
    while (start < length);
#endif
}

// -------------------------------------
// Text window class
// -------------------------------------

chtype CTextWindow::m_cDefaultFocusedColors;
chtype CTextWindow::m_cDefaultDefocusedColors;

CTextWindow::CTextWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, bool wrap, bool follow,
                         char absrel, bool box) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel, box,
                                                                box, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                                                  m_bWrap(wrap), m_bFollow(follow), m_uCurrentLine(0)
{
    if (box)
        m_pTextWin = new CWidgetWindow(this, nlines-2, ncols-2, 1, 1, 'r', false); // Create a new text window
    else
        m_pTextWin = this; // Use current
    
    m_pFMText = new CFormattedText(m_pTextWin, "", m_bWrap);
    
    m_pVScrollbar = new CScrollbar(this, nlines-2, 1, 1, ncols-1, 0, 0, true, 'r');
    m_pHScrollbar = new CScrollbar(this, 1, ncols-2, nlines-1, 1, 0, 0, false, 'r');
}

void CTextWindow::HScroll(int n)
{
    if (m_bWrap || (m_pFMText->GetLongestLine() <= m_pTextWin->width()))
        return;
    
    m_pHScrollbar->Scroll(n);
    refresh();
}

void CTextWindow::VScroll(int n)
{
    if (m_pFMText->Empty())
        return;
    
    m_pVScrollbar->Scroll(n);
    unsigned diff = (unsigned)m_pVScrollbar->GetValue() - m_uCurrentLine;
    
    if (diff)
    {
        m_uCurrentLine = (unsigned)m_pVScrollbar->GetValue();
        refresh();
    }
}

void CTextWindow::ScrollToBottom()
{
    int h = m_pFMText->GetLines() - m_pTextWin->height();
    if (h < 0)
        h = 0;
    
    m_pVScrollbar->SetCurrent(h);
    m_uCurrentLine = (unsigned)m_pVScrollbar->GetValue();
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
    m_pFMText->AddText(text);

    if (m_pFMText->GetLines() > m_pTextWin->height())
        m_pVScrollbar->SetMinMax(0, (m_pFMText->GetLines() - m_pTextWin->height()));
    
    if (m_pFMText->GetLongestLine() > m_pTextWin->width())
        m_pHScrollbar->SetMinMax(0, (m_pFMText->GetLongestLine() - m_pTextWin->width()));
    
    if (m_bFollow)
        ScrollToBottom();
}

void CTextWindow::Clear()
{
    m_pFMText->Clear();
    m_uCurrentLine = 0;
    m_pVScrollbar->SetMinMax(0, 0);
    m_pHScrollbar->SetMinMax(0, 0);
}

void CTextWindow::LoadFile(const char *fname)
{
    std::ifstream file(fname);
    std::string buf;
    char c;
    
    while (file && file.get(c))
        buf += c;
        
    AddText(buf);
}

void CTextWindow::Draw()
{
    erase();
    m_pFMText->Print(m_uCurrentLine, (unsigned)m_pHScrollbar->GetValue(), m_pTextWin->height(), m_pTextWin->width());
    m_pVScrollbar->Enable((HasBox() && (m_pFMText->GetLines() > m_pTextWin->height())));
    m_pHScrollbar->Enable(!m_bWrap && HasBox() && (m_pFMText->GetLongestLine() > m_pTextWin->width()));
}

// -------------------------------------
// Menu class
// -------------------------------------

chtype CMenu::m_cDefaultFocusedColors;
chtype CMenu::m_cDefaultDefocusedColors;

CMenu::CMenu(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
               char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel, true,
                                            true, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                              m_iCursorLine(0), m_iStartEntry(0)
{
    m_pTextWin = new CWidgetWindow(this, nlines-2, ncols-2, 1, 1, 'r', false);
    m_pVScrollbar = new CScrollbar(this, nlines-2, 1, 1, ncols-1, 0, 100, true, 'r');
    m_pHScrollbar = new CScrollbar(this, 1, ncols-2, nlines-1, 1, 0, 100, false, 'r');
}

void CMenu::HScroll(int n)
{
    if (m_uLongestLine <= m_pTextWin->width())
        return;
    
    m_pHScrollbar->Scroll(n);
    refresh();
}

void CMenu::VScroll(int n)
{
    bool scroll = false;
    int newline = m_iCursorLine + n;
    unsigned cur = m_iCursorLine;
    
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
    
    //if (cur != GetCurrent())
    {
        m_MenuItems[cur]->DelRevTag(0, 0);
        m_MenuItems[GetCurrent()]->AddRevTag(0, 0, m_MenuItems[GetCurrent()]->GetText(0).length());
    }

    refresh();
}

bool CMenu::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
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
                std::vector<CFormattedText *>::iterator cur = (m_MenuItems.begin() + GetCurrent());
                
                // First try from current position
                std::vector<CFormattedText *>::iterator it = std::lower_bound(cur+1, m_MenuItems.end(), ch, LowerChar);
                if ((it == m_MenuItems.end()) || ((*it)->GetText(0)[0] != ch))
                    it = std::lower_bound(m_MenuItems.begin(), cur, ch, LowerChar); // Failed, start from the begin
                
                if ((it != m_MenuItems.end()) && ((*it)->GetText(0)[0] == ch))
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
        for(int i=m_iStartEntry; ((i<m_MenuItems.size()) && (lines<m_pTextWin->height())); i++, lines++)
        {
/*            if (m_iCursorLine == lines)
                m_pTextWin->attron(A_REVERSE);*/
            
            m_MenuItems[i]->Print(0, (unsigned)m_pHScrollbar->GetValue(), 1, m_pTextWin->width());
/*            m_pTextWin->AddStrFormat(lines, 0, m_MenuItems[i].c_str(), (int)m_pHScrollbar->GetValue(),
                                     m_pTextWin->width());*/
            
/*            if (m_iCursorLine == lines)
                m_pTextWin->attroff(A_REVERSE);*/
        }
    }
}

void CMenu::AddItem(std::string s)
{
    // Search a place that won't mess up sorted vector
    std::vector<CFormattedText *>::iterator it = std::lower_bound(m_MenuItems.begin(), m_MenuItems.end(), s, SortMenu);
    
    if ((it == m_MenuItems.end()) || ((*it)->GetText(0) != s)) // Don't add duplicates
        m_MenuItems.insert(it, new CFormattedText(m_pTextWin, s, false));
    
    int h = m_MenuItems.size() - m_pTextWin->height();
    if (h < 0)
        h = 0;
    m_pVScrollbar->SetMinMax(0, h);

    unsigned len = GetUnFormatLen(s);
    if (len > m_uLongestLine)
    {
        m_uLongestLine = len;
        
        if (m_uLongestLine > m_pTextWin->width())
            m_pHScrollbar->SetMinMax(0, (m_uLongestLine - m_pTextWin->width()));
    }
    
    m_pVScrollbar->Enable((m_MenuItems.size() > m_pTextWin->height()));
    m_pHScrollbar->Enable((m_uLongestLine > m_pTextWin->width()));
}

void CMenu::SetCurrent(const std::string &str)
{
    std::vector<CFormattedText *>::iterator it; std::find_if(m_MenuItems.begin(), m_MenuItems.end(), CFindMenu(str));
    
    if (it != m_MenuItems.end())
    {
        std::vector<CFormattedText *>::iterator cur = (m_MenuItems.begin() + GetCurrent());
        VScroll(std::distance(cur, it));
        PushEvent(EVENT_DATACHANGED);
    }
    else
        ;// UNDONE: Exception?
}

void CMenu::Clear()
{
    m_MenuItems.clear();
    m_iCursorLine = m_iStartEntry = m_uLongestLine = 0;
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
                                                              absrel, false, true, m_cDefaultFocusedColors,
                                                              m_cDefaultDefocusedColors),
                                                m_chOutChar(out), m_iMaxChars(max), m_iCursorPos(0),
                                                m_iScrollOffset(0)
{
}

void CInputField::Addch(chtype ch)
{
    if ((m_iMaxChars != -1) && (m_szText.length() >= m_iMaxChars))
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
    
    if (pos <= m_szText.length())
        m_szText.erase(pos, 1);
    
    if (backspace)
        MoveCursor(-1);
    else if (pos <= m_szText.length())
    {
        if (m_iScrollOffset)
            m_iScrollOffset--;
        refresh(); // MoveCursor would call it otherwise
    }
}

void CInputField::MoveCursor(int n, bool relative)
{
    int w = width()-1; // -1, because we want to scroll before char is at last available position
    
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

void CInputField::Focus()
{
    CWidgetWindow::Focus();
    curs_set(1);
    SetCursorPos(begy(), begx()+m_iCursorPos);
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
    erase();

    std::string out;

    if (!m_szText.empty())
    {
        if (m_chOutChar)
            out.append(m_szText.length(), m_chOutChar);
        else
            out = m_szText;
    }
    
    out.append(width(), '_');
    AddStrFormat(0, 0, out.c_str(), m_iScrollOffset, width());

    if (Focused())
        SetCursorPos(begy(), begx() + m_iCursorPos);
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
// Checkbox class
// -------------------------------------

chtype CCheckbox::m_cDefaultFocusedColors;
chtype CCheckbox::m_cDefaultDefocusedColors;

CCheckbox::CCheckbox(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                     char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel,
                     false, true, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                     m_ulCheckedboxes(0), m_iSelectedButton(1)
{
}

bool CCheckbox::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
    switch (ch)
    {
        case KEY_UP:
            m_iSelectedButton--;
            if (m_iSelectedButton < 1)
                m_iSelectedButton = m_BoxList.size();
            break;
        case KEY_DOWN:
            m_iSelectedButton++;
            if (m_iSelectedButton > m_BoxList.size())
                m_iSelectedButton = 1;
            break;
        case KEY_ENTER:
        case '\n':
        case '\r':
        {
            PushEvent(EVENT_CALLBACK);
            break;
        }
        case ' ':
            if (IsEnabled(m_iSelectedButton))
                DisableBox(m_iSelectedButton);
            else
                EnableBox(m_iSelectedButton);
            PushEvent(EVENT_DATACHANGED);
            break;
        default:
            handled = false;
            break;
    }
    
    if (handled)
        refresh();
    
    return handled;
}

void CCheckbox::Draw()
{
    int counter = 1;
    for (std::list<std::string>::iterator it=m_BoxList.begin(); it!=m_BoxList.end(); it++)
    {
        std::string out;
        if (IsEnabled(counter))
            out = "[X] ";
        else
            out = "[ ] ";
        
        out += *it;
        
        if (counter == m_iSelectedButton)
        {
            out.insert(0, "<rev>");
            out += "</rev>";
        }
        
        AddStrFormat(counter-1, 0, out);
        counter++;
    }
}

// -------------------------------------
// Radio button class
// -------------------------------------

chtype CRadioButton::m_cDefaultFocusedColors;
chtype CRadioButton::m_cDefaultDefocusedColors;

CRadioButton::CRadioButton(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                           char absrel) : CWidgetWindow(owner, nlines, ncols, begin_y, begin_x, absrel,
                           false, true, m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                           m_iCheckedButton(1), m_iSelectedButton(1)
{
}

bool CRadioButton::HandleKey(chtype ch)
{
    if (CWidgetWindow::HandleKey(ch))
        return true;
    
    bool handled = true;
    
    switch (ch)
    {
        case KEY_UP:
            m_iSelectedButton--;
            if (m_iSelectedButton < 1)
                m_iSelectedButton = m_ButtonList.size();
            break;
        case KEY_DOWN:
            m_iSelectedButton++;
            if (m_iSelectedButton > m_ButtonList.size())
                m_iSelectedButton = 1;
            break;
        case KEY_ENTER:
        case '\n':
        case '\r':
        {
            PushEvent(EVENT_CALLBACK);
            break;
        }
        case ' ':
            EnableButton(m_iSelectedButton);
            PushEvent(EVENT_DATACHANGED);
            break;
        default:
            handled = false;
            break;
    }
    
    if (handled)
        refresh();
    
    return handled;
}

void CRadioButton::Draw()
{
    int counter = 1;
    for (std::list<std::string>::iterator it=m_ButtonList.begin(); it!=m_ButtonList.end(); it++)
    {
        std::string out;
        if (counter == EnabledButton())
            out = "(+) ";
        else
            out = "( ) ";
        
        out += *it;
        
        if (counter == m_iSelectedButton)
        {
            out.insert(0, "<rev>");
            out += "</rev>";
        }
        
        AddStrFormat(counter-1, 0, out);
        counter++;
    }
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
    if (info)
    {
        m_pLabel = new CTextLabel(this, maxlines-4, ncols-4, 2, 2, 'r');
        m_pLabel->AddText(info);
    }
    else
        m_pLabel = NULL;
}

CWidgetBox::CWidgetBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                       const char *info) : CWidgetWindow(owner, maxlines, ncols, begin_y, begin_x, true),
                                           m_bFinished(false), m_bCanceled(false), m_pWidgetManager(owner)
{
    if (info)
    {
        m_pLabel = new CTextLabel(this, maxlines-4, ncols-4, 2, 2, 'r');
        m_pLabel->AddText(info);
    }
    else
        m_pLabel = NULL;
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
    else
        m_pButtons[2] = NULL;
    
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
    int y = (m_pLabel->rely()+m_pLabel->maxy()+2);
    m_pTextField = new CInputField(this, 1, ncols-4, y, 2, 'r', max, out);
    
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
    
    if (m_bCanceled)
        m_pTextField->SetText("");
    
    return m_pTextField->GetText();
}

// -------------------------------------
// File dialog class
// -------------------------------------

chtype CFileDialog::m_cDefaultFocusedColors;
chtype CFileDialog::m_cDefaultDefocusedColors;

CFileDialog::CFileDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *s,
                         const char *info) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x, info,
                                                        m_cDefaultFocusedColors, m_cDefaultDefocusedColors),
                                             m_szStartDir(s), m_szSelectedDir(s), m_szInfo(info)
{
    m_pFileMenu = new CMenu(this, maxlines-10, ncols-4, (m_pLabel->rely()+m_pLabel->maxy()+2), 2, 'r');
    
    m_pFileField = new CInputField(this, 1, ncols-4, (m_pFileMenu->rely()+m_pFileMenu->maxy()+2), 2, 'r');
    m_pFileField->SetText(m_szStartDir);
    
    const int startx = ((ncols + 2) - (2 * 20 + 2)) / 2;
    const int starty = (m_pFileField->rely()+m_pFileField->maxy()+2);
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
    
    if (m_bCanceled)
        m_szSelectedDir = "";
    
    return m_szSelectedDir;
}

// -------------------------------------
// Menu dialog class
// -------------------------------------

chtype CMenuDialog::m_cDefaultFocusedColors;
chtype CMenuDialog::m_cDefaultDefocusedColors;

CMenuDialog::CMenuDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
                         const char *info) : CWidgetBox(owner, maxlines, ncols, begin_y, begin_x, info,
                                             m_cDefaultFocusedColors, m_cDefaultDefocusedColors)
{
    int y = (m_pLabel->rely()+m_pLabel->maxy()+2);
    const int menuh = maxlines - y - 4;
    m_pMenu = new CMenu(this, menuh, ncols-4, y, 2, 'r');
    
    int x = ((ncols + 2) - (2 * 20 + 2)) / 2;
    y += (menuh + 1);
    m_pOKButton = new CButton(this, 1, 20, y, x, "OK", 'r');
    
    m_pCancelButton = new CButton(this, 1, 20, y, x+22, "Cancel", 'r');
    
    ActivateChild(m_pMenu);
    
    Fit(m_pOKButton->rely()+m_pOKButton->maxy()+2);
}

bool CMenuDialog::HandleEvent(CWidgetHandler *p, int type)
{
    if (p == m_pMenu)
    {
        if (type == EVENT_CALLBACK)
        {
            m_bFinished = true;
            m_bCanceled = false;
            m_szSelection = m_pMenu->GetCurrentItemName();
            return true;
        }
    }
    else if (p == m_pOKButton)
    {
        m_bFinished = true;
        m_bCanceled = false;
        m_szSelection = m_pMenu->GetCurrentItemName();
        return true;
    }
    else if (p == m_pCancelButton)
    {
        m_bFinished = m_bCanceled = true;
        m_szSelection = "";
        return true;
    }
    
    return false;
}

const std::string &CMenuDialog::Run()
{
    while (m_pWidgetManager->Run() && !m_bFinished)
        ;
    
    return m_szSelection;
}
