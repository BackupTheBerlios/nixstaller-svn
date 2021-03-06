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

#ifndef CDK_WIDGET_H
#define CDK_WIDGET_H

// CDK Wrapper Classes

class CBaseCDKWidget
{
protected:
    short m_sBackColor;
    bool m_bBox;

public:
    CBaseCDKWidget(bool b) : m_bBox(b) { };
    virtual ~CBaseCDKWidget(void) { Destroy(); };

    virtual void Destroy(void) { UnSetBgColor(); };
    virtual void Draw(void) = 0;
    virtual void SetBgColor(int color) = 0;
    virtual void UnSetBgColor() { };
    virtual EExitType ExitType(void) { return vNORMAL; };
    virtual void Bind(chtype key, BINDFN function, void *data=NULL) = 0;
};

class CCDKLabel: public CBaseCDKWidget
{
    CDKLABEL *m_pLabel;

public:
    CCDKLabel(CDKSCREEN *pScreen, int x, int y, char **msg, int count, bool box=true, bool shadow=false);
    CCDKLabel(CDKSCREEN *pScreen, int x, int y, const std::string &msg, bool box=true, bool shadow=false);
    CCDKLabel(CDKSCREEN *pScreen, int x, int y, const char *msg, bool box=true, bool shadow=false);
    virtual ~CCDKLabel(void) { Destroy(); };

    virtual void Draw(void) { drawCDKLabel(m_pLabel, m_bBox); };
    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKLabelBackgroundColor(m_pLabel, CreateText("</B/%d>", color));
                                         m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKLabelBackgroundColor(m_pLabel, CreateText("<!%d!B>", m_sBackColor)); };
    virtual void Bind(chtype key, BINDFN function, void *data=NULL) { bindCDKObject(vLABEL, m_pLabel, key, function, data); };
    
    CDKLABEL *GetLabel(void) { return m_pLabel; };
    void SetText(char **msg, int count) { setCDKLabelMessage(m_pLabel, msg, count); };
    void SetText(const std::string &msg) { char *sz[1] = { const_cast<char*>(msg.c_str()) };
                                           setCDKLabelMessage(m_pLabel, sz, 1); };
    void SetText(const char *msg) { char *sz[1] = { const_cast<char*>(msg) }; setCDKLabelMessage(m_pLabel, sz, 1); };
};

class CCDKButtonBox: public CBaseCDKWidget
{
    CDKBUTTONBOX *m_pBBox;
    
public:
    CCDKButtonBox(CDKSCREEN *pScreen, int x, int y, int h, int w, char *title, int rows, int cols, char **buttons,
                  int count, chtype hlight=A_REVERSE, bool box=true, bool shadow=false);
    virtual ~CCDKButtonBox(void) { Destroy(); };

    virtual void Draw(void) { drawCDKButtonbox(m_pBBox, m_bBox); };
    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKButtonboxBackgroundColor(m_pBBox, CreateText("</B/%d>", color));
                                         m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKButtonboxBackgroundColor(m_pBBox, CreateText("<!%d!B>", m_sBackColor)); };
    virtual void Bind(chtype key, BINDFN function, void *data=NULL) { bindCDKObject(vBUTTONBOX, m_pBBox, key,
                                                                      function, data); };
    
    CDKBUTTONBOX *GetBBox(void) { return m_pBBox; };
    int GetCurrent(void) { return m_pBBox->currentButton; };
};

class CCDKScroll: public CBaseCDKWidget
{
    CDKSCROLL *m_pScroll;

public:
    CCDKScroll(CDKSCREEN *pScreen, int x, int y, int h, int w, int sbpos, char *title, char **list, int lcount,
               bool box=true, bool numbers=false, bool shadow=false);
    virtual ~CCDKScroll(void) { Destroy(); };

    virtual void Draw(void) { drawCDKScroll(m_pScroll, m_bBox); };
    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKScrollBackgroundColor(m_pScroll, CreateText("</B/%d>", color));
                                         m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKScrollBackgroundColor(m_pScroll, CreateText("<!%d!B>", m_sBackColor)); };
    virtual EExitType ExitType(void) { return m_pScroll->exitType; };
    virtual void Bind(chtype key, BINDFN function, void *data=NULL) { bindCDKObject(vSCROLL, m_pScroll, key, function, data); };
    
    int Activate(chtype *actions = NULL) { return activateCDKScroll(m_pScroll, actions); };
    CDKSCROLL *GetScroll(void) { return m_pScroll; };
    int GetCurrent(void) { return getCDKScrollCurrent(m_pScroll); };
    void SetCurrent(int n) { setCDKScrollCurrent(m_pScroll, n); };
};

class CCDKAlphaList: public CBaseCDKWidget
{
    CDKALPHALIST *m_pAList;

public:
    CCDKAlphaList(CDKSCREEN *pScreen, int x, int y, int h, int w, char *title, char *label, char **list, int count,
                  bool box=true, bool shadow=false);
    virtual ~CCDKAlphaList(void) { Destroy(); };

    virtual void Draw(void) { drawCDKAlphalist(m_pAList, m_bBox); };
    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKAlphalistBackgroundColor(m_pAList, CreateText("</B/%d>", color));
                                         m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKAlphalistBackgroundColor(m_pAList, CreateText("<!%d!B>", m_sBackColor)); };
    virtual EExitType ExitType(void) { return m_pAList->exitType; };
    virtual void Bind(chtype key, BINDFN function, void *data=NULL) { bindCDKObject(vALPHALIST, m_pAList, key, function, data); };
    
    char *Activate(chtype *actions = NULL) { return activateCDKAlphalist(m_pAList, actions); };
    CDKALPHALIST *GetAList(void) { return m_pAList; };
    void SetContent(char **list, int count) { setCDKAlphalistContents(m_pAList, list, count); };
};

//class CCharListHelper;

class CCDKDialog: public CBaseCDKWidget
{
    CDKDIALOG *m_pDialog;

    void SetButtonBar(void);
    
public:
    CCDKDialog(CDKSCREEN *pScreen, int x, int y, char **message, int mcount, char **buttons, int bcount,
               chtype hlight=COLOR_PAIR(2)|A_REVERSE, bool sep=true, bool box=true, bool shadow=false);
    CCDKDialog(CDKSCREEN *pScreen, int x, int y, const char *message, char **buttons, int bcount,
               chtype hlight=COLOR_PAIR(2)|A_REVERSE, bool sep=true, bool box=true, bool shadow=false);
    CCDKDialog(CDKSCREEN *pScreen, int x, int y, const std::string &message, char **buttons, int bcount,
               chtype hlight=COLOR_PAIR(2)|A_REVERSE, bool sep=true, bool box=true, bool shadow=false);
    virtual ~CCDKDialog(void) { Destroy(); };
    
    virtual void Draw(void) { drawCDKDialog(m_pDialog, m_bBox); };
    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKDialogBackgroundColor(m_pDialog, CreateText("</B/%d>", color));
                                         m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKDialogBackgroundColor(m_pDialog, CreateText("<!%d!B>", m_sBackColor)); };
    virtual EExitType ExitType(void) { return m_pDialog->exitType; };
    virtual void Bind(chtype key, BINDFN function, void *data=NULL) { bindCDKObject(vDIALOG, m_pDialog, key, function, data); };
    
    int Activate(chtype *actions = NULL);
    CDKDIALOG *GetDialog(void) { return m_pDialog; };
};

class CCDKSWindow: public CBaseCDKWidget
{
    CDKSWINDOW *m_pSWindow;
    
public:
    CCDKSWindow(CDKSCREEN *pScreen, int x, int y, int h, int w, char *title, int slines, bool box=true, bool shadow=false);
    virtual ~CCDKSWindow(void) { Destroy(); };
    
    virtual void Draw(void) { drawCDKSwindow(m_pSWindow, m_bBox); };
    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKSwindowBackgroundColor(m_pSWindow, CreateText("</B/%d>", color));
                                         m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKSwindowBackgroundColor(m_pSWindow, CreateText("<!%d!B>", m_sBackColor)); };
    virtual void Bind(chtype key, BINDFN function, void *data=NULL) { bindCDKObject(vSWINDOW, m_pSWindow, key, function, data); };
    
    void AddText(const std::string &txt, bool wrap=true, int pos=BOTTOM) { AddText(const_cast<char *>(txt.c_str()),
                                                                                   wrap, pos); };
    void AddText(const char *txt, bool wrap=true, int pos=BOTTOM,
                 int maxch=-1) { AddText(const_cast<char *>(txt), wrap, pos, maxch); };
    void AddText(char *str, bool wrap=true, int pos=BOTTOM, int maxch=-1);
    
    int Exec(const char *command, int pos=BOTTOM) { return Exec(const_cast<char *>(command), pos); };
    int Exec(const std::string &command, int pos=BOTTOM) { return Exec(const_cast<char *>(command.c_str()), pos); };
    int Exec(char *command, int pos=BOTTOM) { return execCDKSwindow(m_pSWindow, command, pos); };

    void Activate(chtype *actions = NULL) { activateCDKSwindow(m_pSWindow, actions); };
    CDKSWINDOW *GetSWin(void) { return m_pSWindow; };
    void Clear(void) { cleanCDKSwindow(m_pSWindow); };
};

class CCDKEntry: public CBaseCDKWidget
{
    CDKENTRY *m_pEntry;
    
public:
    CCDKEntry(CDKSCREEN *pScreen, int x, int y, char *title, char *label, int fwidth, int min, int max,
              EDisplayType DispType=vMIXED, chtype fillch='.', chtype fieldattr=A_NORMAL, bool box=true, bool shadow=false);
    virtual ~CCDKEntry(void) { Destroy(); };
    
    virtual void Draw(void) { drawCDKEntry(m_pEntry, m_bBox); };
    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKEntryBackgroundColor(m_pEntry, CreateText("</B/%d>", color));
                                         m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKEntryBackgroundColor(m_pEntry, CreateText("<!%d!B>", m_sBackColor)); };
    virtual EExitType ExitType(void) { return m_pEntry->exitType; };
    virtual void Bind(chtype key, BINDFN function, void *data=NULL) { bindCDKObject(vENTRY, m_pEntry, key, function, data); };
    
    void SetHiddenChar(chtype ch) { setCDKEntryHiddenChar(m_pEntry, ch); };
    
    void SetValue(std::string &str) { SetValue(const_cast<char*>(str.c_str())); };
    void SetValue(const char *str) { SetValue(const_cast<char*>(str)); };
    void SetValue(char *str) { setCDKEntryValue(m_pEntry, str); };

    void Clean(void) { cleanCDKEntry(m_pEntry); };
    
    char *Activate(chtype *actions = NULL);
    CDKENTRY *GetEntry(void) { return m_pEntry; };
};

class CCDKHistogram: public CBaseCDKWidget
{
    CDKHISTOGRAM *m_pHistogram;
    
public:
    CCDKHistogram(CDKSCREEN *pScreen, int x, int y, int h, int w, int orient, char *title, bool box=true, bool shadow=false);
    virtual ~CCDKHistogram(void) { Destroy(); };
    
    virtual void Draw(void) { drawCDKHistogram(m_pHistogram, m_bBox); };
    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKHistogramBackgroundColor(m_pHistogram, CreateText("</B/%d>", color));
                                         m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKHistogramBackgroundColor(m_pHistogram, CreateText("<!%d!B>", m_sBackColor)); };
    virtual void Bind(chtype key, BINDFN function, void *data=NULL) { bindCDKObject(vHISTOGRAM, m_pHistogram, key, function,
                                                                      data); };
    
    void SetHistogram(EHistogramDisplayType vtype, int statspos, int min, int max, int cur, chtype fillch,
                      chtype statattr=A_BOLD);
    void SetValue(int min, int max, int cur) { setCDKHistogramValue(m_pHistogram, min, max, cur); };
    CDKHISTOGRAM *GetHistogram(void) { return m_pHistogram; };
};

// CDK Wrapper Ends here

class CButtonBar
{
    struct bar_entry_s
    {
        std::string szCurrentText;
        int iCurTextLength;
        std::vector<char *> Texts;

        bar_entry_s(void) : iCurTextLength(0) { };
    };

    bar_entry_s *m_pCurrentBarEntry;
    std::list<bar_entry_s *> m_ButtonBarEntries;
    CCDKLabel *m_pLabel;
    
public:
    CButtonBar(void) : m_pCurrentBarEntry(0), m_pLabel(NULL) { };
    ~CButtonBar(void) { Destroy(); };

    void Push(void); // Creates new button bar
    void Pop(void); // Destroys current bar and sets previous one as current
    void AddButton(const char *button, const char *desc);
    void Clear(void) { if (m_pCurrentBarEntry) m_pCurrentBarEntry->Texts.clear(); };
    void Draw(void);
    void Destroy(void) { delete m_pLabel; m_pLabel = NULL; };
    CCDKLabel *GetLabel(void) { return m_pLabel; };
};

class CFileDialog
{
    std::string m_szStartDir, m_szDestDir, m_szTitle;
    CCDKAlphaList *m_pFileList;
    CCDKSWindow *m_pCurDirWin;
    std::vector<char *> m_DirItems; // Vector of all sub directories in current dir
    bool m_bRestoreDir, m_bAskWAccess;

    bool ReadDir(const std::string &dir);
    void UpdateCurDirText(void);
    void ClearDirList(void);
    
public:
    CFileDialog(const std::string &s, const std::string &t, bool r, bool w) : m_szStartDir(s), m_szTitle(t),
                                                                              m_pFileList(NULL), m_pCurDirWin(NULL),
                                                                              m_bRestoreDir(r), m_bAskWAccess(w) { };
    ~CFileDialog(void) { Destroy(); };

    bool Activate(void);
    const char *Result(void) { return m_szDestDir.c_str(); };
    void Destroy(void);

    bool UpdateFileList(const char *dir);

    static int CreateDirCB(EObjectType cdktype GCC_UNUSED, void *object, void *clientData, chtype key);
};

#endif
