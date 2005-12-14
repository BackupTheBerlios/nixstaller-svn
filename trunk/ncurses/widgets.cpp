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
    m_CharList.AddItem(message);
    m_pDialog = newCDKDialog(pScreen, x, y, m_CharList, m_CharList.Count(), buttons, bcount, hlight, sep, box, shadow);
    if (!m_pDialog) throwerror(false, "Could not create dialog window");
}

CCDKDialog::CCDKDialog(CDKSCREEN *pScreen, int x, int y, const std::string &message, char **buttons, int bcount,
                       chtype hlight, bool sep, bool box, bool shadow) : CBaseCDKWidget(box)
{
    m_CharList.AddItem(message);
    m_pDialog = newCDKDialog(pScreen, x, y, m_CharList, m_CharList.Count(), buttons, bcount, hlight, sep, box, shadow);
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
        std::istringstream istr(txt);
        std::string line, tmpstr;

        istr >> line; // Need atleast one word...
        while(istr >> tmpstr)
        {
            if ((line.length() + tmpstr.length() + 1) > maxch)
            {
                addCDKSwindow(m_pSWindow, const_cast<char *>(line.c_str()), BOTTOM);
                line = tmpstr;
            }
            else
                line += " " + tmpstr;
        }
        addCDKSwindow(m_pSWindow, const_cast<char *>(line.c_str()), pos);
    }
    else
        addCDKSwindow(m_pSWindow, txt, pos);
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
        m_pCurrentBarEntry->Texts.AddItem(m_pCurrentBarEntry->szCurrentText);
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
        m_pCurrentBarEntry->Texts.AddItem(m_pCurrentBarEntry->szCurrentText);
        m_pCurrentBarEntry->szCurrentText.clear();
        m_pCurrentBarEntry->iCurTextLength = 0;
    }
    
    if (!m_pCurrentBarEntry->Texts.Count()) return;

    if (m_pLabel) delete m_pLabel; // CDK has no propper way to change text...
    
    m_pLabel = new CCDKLabel(CDKScreen, CENTER, BOTTOM, m_pCurrentBarEntry->Texts, m_pCurrentBarEntry->Texts.Count());
    if (!m_pLabel)
        throwerror(false, "Could not create button bar");
    m_pLabel->SetBgColor(24);
    
    m_pLabel->Draw();
    refreshCDKScreen(CDKScreen);
}

// File select dialog

bool CFileDialog::ReadDir(const std::string &dir, CCharListHelper *Items)
{
    struct dirent *dirstruct;
    struct stat filestat;
    DIR *dp;
    
    dp = opendir (dir.c_str());
    if (dp == 0) return false;

    while ((dirstruct = readdir (dp)) != 0)
    {
        if ((lstat(dirstruct->d_name, &filestat) == 0) && (mode2Filetype(filestat.st_mode) == 'd') &&
             (strcmp(dirstruct->d_name, ".")))
            Items->AddItem(dirstruct->d_name);
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

bool CFileDialog::Activate()
{
    char *buttons[] = { GetTranslation("Open directory"), GetTranslation("Select directory"), GetTranslation("Cancel") };
    char label[] = "Dir: ";
    char curdir[1024];
    CCharListHelper Items;
    
    ButtonBar.Push();
    ButtonBar.AddButton("TAB", "Next button");
    ButtonBar.AddButton("ENTER", "Activate button");
    ButtonBar.AddButton("Arrows", "Navigate menu");
    ButtonBar.AddButton("C", "Create directory");
    ButtonBar.AddButton("ESC", "Cancel");
    ButtonBar.Draw();

    if (!getcwd(curdir, sizeof(curdir))) throwerror(true, "Could not read current directory");
    
    if (chdir(m_szStartDir.c_str()) != 0)
        throwerror(true, "Couldn't open directory '%s'", m_szStartDir.c_str());

    if (!ReadDir(m_szStartDir, &Items)) throwerror(true, "Could not read directory %s", m_szStartDir.c_str());
    
    CCDKButtonBox ButtonBox(CDKScreen, CENTER, GetMaxHeight()-2, 1, 49, 0, 1, 3, buttons, 3);
    ButtonBox.SetBgColor(5);

    m_pCurDirWin = new CCDKSWindow(CDKScreen, CENTER, getbegy(ButtonBox.GetBBox()->win)-2, 2, DEFAULT_WIDTH, NULL, 1);
    m_pCurDirWin->SetBgColor(5);

    m_pFileList = new CCDKAlphaList(CDKScreen, CENTER, 2, getbegy(m_pCurDirWin->GetSWin()->win)-1, DEFAULT_WIDTH,
                                    const_cast<char*>(m_szTitle.c_str()), label, Items, Items.Count());
    m_pFileList->SetBgColor(5);
    setCDKEntryPreProcess(m_pFileList->GetAList()->entryField, CreateDirCB, this);
    m_pFileList->GetAList()->entryField->dispType = vVIEWONLY;  // HACK: Disable backspace

    setCDKAlphalistLLChar(m_pFileList->GetAList(), ACS_LTEE);
    setCDKAlphalistLRChar(m_pFileList->GetAList(), ACS_RTEE);
    setCDKLabelLLChar(m_pCurDirWin->GetSWin(), ACS_LTEE);
    setCDKLabelLRChar(m_pCurDirWin->GetSWin(), ACS_RTEE);
    setCDKButtonboxULChar(ButtonBox.GetBBox(), ACS_LTEE);
    setCDKButtonboxURChar(ButtonBox.GetBBox(), ACS_RTEE);
    
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
        if (ButtonBox.GetCurrent() == 1) break;
        if (!selection || !selection[0]) continue;

        if (!WriteAccess(selection))
        {
            WarningBox("You don't have write access for this directory");
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
    struct dirent *dirstruct;
    struct stat filestat;
    DIR *dp;
    CCharListHelper Items;
    
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
    if (!ReadDir(m_szDestDir, &Items)) { WarningBox("Could not read directory"); return false; }
    
    m_pFileList->SetContent(Items, Items.Count());
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
}

int CFileDialog::CreateDirCB(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED, void *clientData, chtype key)
{
    if (key != 'c') return true;
    
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
        WarningBox("%s\n%.75s\n%.75s", GetTranslation("Could not create the directory"), newdir, strerror(errno));
        return false;
    }

    CFileDialog *FileList = (CFileDialog *)clientData;
    FileList->UpdateFileList(newdir);
    return false;
}
