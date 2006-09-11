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

#ifndef WIDGET_H
#define WIDGET_H

#ifdef REMOVE_ME_AFTER_NEW_FRONTEND_IS_DONE

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

#include <set>
#include <stack>

class CWidgetWindow;

class CWidgetHandler
{
    bool m_bEnabled;
    bool m_bFocused, m_bCanFocus;
    CWidgetHandler *m_pBoundKeyWidget; // Widgets which will recieve key events from this widget
    
    friend class CWidgetManager;

protected:
    CWidgetHandler *m_pOwner;
    bool m_bDeleteMe; // Delete it later, incase we are in a loop from ie Run()

    std::list<CWidgetWindow *> m_ChildList;
    std::list<CWidgetWindow *>::iterator m_FocusedChild;

    bool SetNextWidget(bool rec=true); // rec(ursive): If the next widget of all child widgets should be enabled 
    bool SetPrevWidget(bool rec=true);
    
    virtual void Focus(void);
    virtual void LeaveFocus(void);
    virtual bool HandleKey(chtype ch);
    virtual bool HandleEvent(CWidgetHandler *p, int type) { return false; };
    
    void PushEvent(int type);

    CWidgetHandler(CWidgetHandler *owner, bool canfocus=true) : m_bEnabled(true), m_bFocused(false),
                                                                m_bCanFocus(canfocus), m_pBoundKeyWidget(NULL),
                                                                m_pOwner(owner), m_bDeleteMe(false),
                                                                m_FocusedChild(m_ChildList.end()) { };

public:
    enum { EVENT_CALLBACK, EVENT_DATACHANGED };

    virtual ~CWidgetHandler(void);
    
    void AddChild(CWidgetWindow *p);
    virtual void RemoveChild(CWidgetWindow *p);
    
    virtual void ActivateChild(CWidgetWindow *p);
    
    bool Enabled(void) { return m_bEnabled; };
    virtual void Enable(bool e);
    
    bool Focused(void) { return m_bFocused; };
    bool CanFocus(void) { return m_bCanFocus; };
    void SetCanFocus(bool f) { m_bCanFocus = f; };
    
    void BindKeyWidget(CWidgetHandler *p) { m_pBoundKeyWidget = p; };
};

class CButtonBar;

class CWidgetManager: public CWidgetHandler
{
    bool m_bQuit;
    
protected:
    bool SetNextChildWidget(void);
    bool SetPrevChildWidget(void);

public:
    static CButtonBar *m_pButtonBar;
    
    CWidgetManager(void) : CWidgetHandler(NULL, false), m_bQuit(false) { };
    
    void Init(void);
    void Refresh(void);
    virtual void RemoveChild(CWidgetWindow *p);
    virtual void ActivateChild(CWidgetWindow *p);
    
    bool Run(void);
};

class CFormattedText
{
protected:
    struct line_entry_s
    {
        std::string text;
        line_entry_s(const std::string &s) : text(s) { };
        line_entry_s(void) { };
    };
    
    struct color_entry_s
    {
        unsigned count;
        int fgcolor, bgcolor;
        color_entry_s(unsigned cn, int fg, int bg) : count(cn), fgcolor(fg), bgcolor(bg) { };
    };

    typedef std::map<unsigned, color_entry_s *> TColorTagList; // Index is used for starting position
    typedef std::map<unsigned, unsigned> TRevTagList; // Index is the starting position, char count is the value
    typedef std::set<unsigned> TCenterTagList;

    std::vector<line_entry_s *> m_Lines;
    TColorTagList m_ColorTags;
    TRevTagList m_ReversedTags;
    TCenterTagList m_CenteredIndexes;

private:
    CWidgetWindow *m_pWindow;
    unsigned m_uTextLength;
    unsigned m_uCurrentLine;
    unsigned m_uWidth, m_uMaxHeight, m_uLongestLine;
    bool m_bWrap, m_bHandleTags;
    
    TColorTagList::iterator GetNextColorTag(TColorTagList::iterator cur, unsigned curpos);
    TRevTagList::iterator GetNextRevTag(TRevTagList::iterator cur, unsigned curpos);
    
public:
    CFormattedText(CWidgetWindow *w, const std::string &str, bool wrap, unsigned maxh=std::numeric_limits<unsigned>::max());
    virtual ~CFormattedText(void) { Clear(); };

    static unsigned CalcLines(const std::string &str, bool wrap, unsigned width);
    
    virtual void AddText(const std::string &str);
    void SetText(const std::string &str) { Clear(); AddText(str); };
    void Print(unsigned startline=0, unsigned startw=0, unsigned endline=std::numeric_limits<unsigned>::max(),
               unsigned endw=std::numeric_limits<unsigned>::max());
    unsigned GetLines(void);
    unsigned GetLongestLine(void) { return m_uLongestLine; };
    const std::string &GetText(unsigned line) { return m_Lines[line]->text; };
    bool Empty(void) { return !GetLines(); };
    void Clear(void);
    
    void AddCenterTag(unsigned line);
    void AddRevTag(unsigned line, unsigned pos, unsigned c);
    void AddColorTag(unsigned line, unsigned pos, unsigned c, int fg, int bg);
    
    void DelCenterTag(unsigned line);
    void DelRevTag(unsigned line, unsigned pos);
    void DelColorTag(unsigned line, unsigned pos);
};

class CWidgetWindow: public CWidgetHandler, public NCursesWindow
{
public:
    typedef std::map<int, std::map<int, int> > ColorMapType;

private:
    bool m_bBox, m_bInitBBar;
    short m_sCurColor; // Current color pair used in formatted text
    chtype m_cLLCorner, m_cLRCorner, m_cULCorner, m_cURCorner;
    chtype m_cFocusedColors, m_cDefocusedColors;
    std::list<std::pair<const char *, const char *> > m_Buttons; // Buttons for button bar

    friend class CWidgetHandler;
    friend class CWidgetManager;

    static ColorMapType m_ColorPairs; // Map for easy getting color pairs
    static int m_iCurColorPair;
    static int m_iCursorY, m_iCursorX;
    
protected:
    std::string m_szTitle;
    
    virtual void Focus(void);
    virtual void LeaveFocus(void);
    virtual void Draw(void) { };
    virtual void SetButtonBar(void);
    
    void AddButton(const char *button, const char *desc) { m_Buttons.push_back(std::pair<const char *, const char *>(button, desc)); };
    
    int Box(void) { return ::wborder(w, 0, 0, 0, 0, m_cULCorner, m_cURCorner, m_cLLCorner, m_cLRCorner); };
    
    static void SetCursorPos(int y, int x) { m_iCursorY = y; m_iCursorX = x; }; // Lock cursor position
    static void ReleaseCursor(void) { m_iCursorY = MaxY(); m_iCursorX = MaxX(); };
    static void ApplyCursorPos(void) { ::move(m_iCursorY, m_iCursorX); };
    
    CWidgetWindow(CWidgetManager *owner, int nlines, int ncols, int begin_y, int begin_x,
                  bool box, bool canfocus, chtype fcolor, chtype dfcolor);
    CWidgetWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel,
                  bool box, bool canfocus, chtype fcolor, chtype dfcolor);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;

    CWidgetWindow(CWidgetManager *owner, int nlines, int ncols, int begin_y, int begin_x, bool box=true);
    CWidgetWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel = 'a',
                  bool box=true);

    virtual int refresh();

    bool HasBox(void) { return m_bBox; };
    void SetBox(bool box) { m_bBox = box; };

    void SetColors(chtype f, chtype df) { m_cFocusedColors = f; m_cDefocusedColors = df; bkgd((Focused()) ? f : df); };
    
    void SetLLCorner(chtype c) { m_cLLCorner = c; };
    void SetLRCorner(chtype c) { m_cLRCorner = c; };
    void SetULCorner(chtype c) { m_cULCorner = c; };
    void SetURCorner(chtype c) { m_cURCorner = c; };
    
    void SetTitle(const char *str) { m_szTitle = str; };
    void SetTitle(const std::string &str) { m_szTitle = str; };
    
    int resize(int nlines, int ncols) { return ::wresize(w, nlines, ncols); };
    int relx(void) { return (par) ? (begx() - par->begx()) : begx(); };
    int rely(void) { return (par) ? (begy() - par->begy()) : begy(); };
    
    virtual int mvwin(int begin_y, int begin_x);
    virtual void Enable(bool e) { CWidgetHandler::Enable(e); if (!e) erase(); };
    
    static int GetColorPair(int fg, int bg);
};

class CButton: public CWidgetWindow
{
    std::string m_szTitle;
    CFormattedText m_FMText;
    
protected:
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CButton(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
            const char *text, char absrel = 'a');
    
    void Push(void) { PushEvent(EVENT_CALLBACK); };
    void SetTitle(const std::string &str) { m_FMText.SetText(str); m_FMText.AddCenterTag(0); };
};

class CScrollbar: public CWidgetWindow
{
    float m_fMinVal, m_fMaxVal, m_fCurVal;
    bool m_bVertical; // if not vertical, than it's horizontal...
    
protected:
    virtual void Draw(void);
    
public:
    CScrollbar(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, float min, float max,
               bool vertical, char absrel = 'a');
    
    void SetMinMax(int min, int max) { m_fMinVal = min; m_fMaxVal = max; };
    void SetCurrent(int cur) { m_fCurVal = cur; };
    float GetValue(void) { return m_fCurVal; };
    void Scroll(float n); // Scroll n steps. Negative n is up, positive down.
};

class CTextLabel: public CWidgetWindow
{
    int m_iMaxHeight;
    unsigned m_uCurLines;
    CFormattedText m_FMText;
    
protected:
    virtual void Draw(void);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CTextLabel(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel = 'a');
    
    void AddText(std::string text);
    void SetText(const std::string &text) { m_FMText.Clear(); AddText(text); };
    void SetText(const char *text) { m_FMText.Clear(); AddText(text); };
    
    static unsigned CalcHeight(int ncols, const std::string &text) { return CFormattedText::CalcLines(text, true, ncols); }
    static unsigned CalcHeight(int ncols, const char *text) { return CFormattedText::CalcLines(text, true, ncols); }
};

class CTextWindow: public CWidgetWindow
{
    CScrollbar *m_pVScrollbar, *m_pHScrollbar;
    CFormattedText *m_pFMText;
    CWidgetWindow *m_pTextWin; // Window containing the actual text
    bool m_bWrap, m_bFollow;
    unsigned m_uCurrentLine;

    void ScrollToBottom(void);
    void HScroll(int n);
    void VScroll(int n);
    
protected:
    
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CTextWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, bool wrap, bool follow,
                char absrel = 'a', bool box=true);
    ~CTextWindow(void) { delete m_pFMText; };
    
    void AddText(std::string text);
    void Clear(void);
    void SetText(const std::string &text) { Clear(); AddText(text); };
    void SetText(const char *text) { Clear(); AddText(text); };
    void LoadFile(const char *fname);
};

class CMenu: public CWidgetWindow
{
    class CMenuText: public CFormattedText
    {
        template <typename C> struct unsorted_tag_s
        {
            C entry;
            unsigned lineoffset;
            line_entry_s *line;
            unsorted_tag_s(C c, unsigned l, line_entry_s *pl) : entry(c), lineoffset(l), line(pl) { };
        };
    
        // For sorting, makes sure that empty lines are at the end
        static bool LessThan(line_entry_s *f, line_entry_s *s)
        {
            std::string tmp=f->text;
            EatWhite(tmp);
        
            std::string tmp2=s->text;
            EatWhite(tmp2);

            return tmp2.empty() || (!tmp.empty() && (f->text < s->text));
        };
        
        static bool LessThanStr(line_entry_s *f, const std::string &s)
        {
            std::string tmp=f->text;
            EatWhite(tmp);
        
            std::string tmp2=s;
            EatWhite(tmp2);

            return !tmp2.empty() && (!tmp.empty() && (f->text < s));
        };
    
        static bool LessThanCh(line_entry_s *f, char ch)
        {
            std::string tmp=f->text;
            EatWhite(tmp);

            return (!tmp.empty() && (f->text[0] < ch));
        };
        
    public:
        CMenuText(CWidgetWindow *w) : CFormattedText(w, "", false) { };
        
        virtual void AddText(const std::string &str);
        void HighLight(unsigned line, bool h);
        unsigned Search(unsigned min, unsigned max, const std::string &key);
        unsigned Search(unsigned min, unsigned max, char key);
    };
    
    bool m_bInitCursor; // Does the current line needs to be highlighted?
    int m_iCursorLine;
    int m_iStartEntry; // First entry to display(for scrolling)
    CScrollbar *m_pVScrollbar, *m_pHScrollbar;
    CWidgetWindow *m_pTextWin; // Window containing the actual text
    CMenuText *m_pMenuText;
    
    void HScroll(int n);
    void VScroll(int n);
    int GetCurrent(void) { return m_iStartEntry+m_iCursorLine; };
    
protected:
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CMenu(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel = 'a');
    virtual ~CMenu(void) { Clear(); delete m_pMenuText; };
    
    void AddItem(std::string s, bool tags=false);
    void Clear(void);
    const std::string &GetCurrentItemName(void) { return m_pMenuText->GetText(GetCurrent()); };
    void SetCurrent(const std::string &str);
    void SetCurrent(const char *str) { SetCurrent(std::string(str)); };
    bool Empty(void) { return m_pMenuText->Empty(); };
};

class CInputField: public CWidgetWindow
{
    chtype m_chOutChar;
    std::string m_szText;
    int m_iMaxChars;
    int m_iCursorPos, m_iScrollOffset;
    CFormattedText m_FMText;
    
    void Addch(chtype ch);
    void Delch(bool backspace);
    void MoveCursor(int n, bool relative=true);
    void ChangeFMText(void);
    
protected:
    virtual void Focus(void);
    virtual void LeaveFocus(void) { CWidgetWindow::LeaveFocus(); curs_set(0); ReleaseCursor(); };
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CInputField(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel = 'a',
                int max=-1, chtype out=0);
    
    const std::string &GetText(void) { return m_szText; };
    void SetText(const std::string &s) { m_szText = s; ChangeFMText(); MoveCursor(m_szText.length(), false); };
    void SetText(const char *s) { m_szText = s; ChangeFMText(); MoveCursor(m_szText.length(), false); };
};

class CProgressbar: public CWidgetWindow
{
    float m_fMin, m_fMax, m_fCurrent;

protected:
    virtual void Draw(void);
    
public:
    static chtype m_cDefaultFillColors, m_cDefaultEmptyColors;
    
    CProgressbar(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                 int min, int max, char absrel = 'a');
    
    void SetCurrent(int n) { m_fCurrent = (float)n; };
};

class CCheckbox: public CWidgetWindow
{
    unsigned long m_ulCheckedboxes;
    std::list<std::string> m_BoxList;
    int m_iSelectedButton;

protected:
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);
    virtual void SetButtonBar(void) { debugline("bbar\n"); AddButton("SPACE", "Enable/Disable"); };

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;

    CCheckbox(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
              char absrel = 'a');

    void Add(const char *text) { m_BoxList.push_back(text); };
    void Add(const std::string &text) { m_BoxList.push_back(text); };
    void EnableBox(int n) { m_ulCheckedboxes |= (1<<(n-1)); };
    void DisableBox(int n) { m_ulCheckedboxes &= ~(1<<(n-1)); };
    bool IsEnabled(int n) { return (m_ulCheckedboxes & (1<<(n-1))); };
};

class CRadioButton: public CWidgetWindow
{
    int m_iCheckedButton;
    std::list<std::string> m_ButtonList;
    int m_iSelectedButton;

protected:
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;

    CRadioButton(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                char absrel = 'a');

    void Add(const char *text) { m_ButtonList.push_back(text); };
    void Add(const std::string &text) { m_ButtonList.push_back(text); };
    void EnableButton(int n) { m_iCheckedButton = n; };
    int EnabledButton(void) { return m_iCheckedButton; };
};

class CButtonBar: public CWidgetWindow
{
    typedef std::pair<const char *, const char *> TButtonEntry;
    typedef std::list<TButtonEntry> TButtonList;
    
    CTextLabel *m_pButtonText;
    std::stack<TButtonList> m_ButtonTexts;
    bool m_bDirty; // True when button text has to be set
    
protected:
    virtual void Draw(void);
    
public:
    static chtype m_cDefaultColors;
    
    CButtonBar(CWidgetManager *owner, int nlines, int ncols, int begin_y, int begin_x);
    
    void Clear(void) { m_pButtonText->SetText(""); };
    void Push(void);
    void AddButton(const char *button, const char *desc);
    void Pop(void);
};

class CWidgetBox: public CWidgetWindow
{
protected:
    bool m_bFinished, m_bCanceled;
    CWidgetManager *m_pWidgetManager;
    CTextLabel *m_pLabel; // Derived classes will need coords from this label, so protected

    virtual bool HandleKey(chtype ch);
    void Fit(int nlines); // Fit on screen: Resize and center this window
    
    CWidgetBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
               const char *info, chtype fcolor, chtype dfcolor);
    CWidgetBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x,
               const char *info=NULL);
};

class CMessageBox: public CWidgetBox
{
protected:
    CButton *m_pOKButton;

    virtual bool HandleEvent(CWidgetHandler *p, int type);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CMessageBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *text);
    
    void Run(void) { while (m_pWidgetManager->Run() && !m_bFinished) {}; };
};

class CWarningBox: public CMessageBox
{
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;

    CWarningBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *text);
};

class CYesNoBox: public CWidgetBox
{
    CButton *m_pYesButton, *m_pNoButton;

protected:
    virtual bool HandleEvent(CWidgetHandler *p, int type);

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CYesNoBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *text);

    bool Run(void) { while (m_pWidgetManager->Run() && !m_bFinished); return !m_bCanceled; };
};

class CChoiceBox: public CWidgetBox
{
    CButton *m_pButtons[3];
    int m_iSelectedButton;
    
protected:
    virtual bool HandleEvent(CWidgetHandler *p, int type);

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;

    CChoiceBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *text,
               const char *but1, const char *but2, const char *but3=NULL);

    int Run(void);
};

class CInputDialog: public CWidgetBox
{
    bool m_bSecure; // For passwords; prints stars, clears text buffer
    CInputField *m_pTextField;
    CButton *m_pOKButton, *m_pCancelButton;

protected:
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CInputDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *info,
                 int max=-1, bool sec=false);

    void SetText(const std::string &s) { m_pTextField->SetText(s); };
    void SetText(const char *s) { m_pTextField->SetText(std::string(s)); };
    
    const std::string &Run(void);
};

class CFileDialog: public CWidgetBox // Currently only browses directories
{
    std::string m_szStartDir, m_szSelectedDir, m_szInfo;
    CMenu *m_pFileMenu;
    CInputField *m_pFileField;
    CButton *m_pOpenButton, *m_pCancelButton;
    
    void OpenDir(std::string newdir="");
    void UpdateDirField(void) { m_pFileField->SetText(m_szSelectedDir); };
    
protected:
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    virtual bool HandleKey(chtype ch);
    virtual void SetButtonBar(void);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CFileDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *s,
                const char *i);
    
    const std::string &Run(void);
};

class CMenuDialog: public CWidgetBox
{
    CMenu *m_pMenu;
    CButton *m_pOKButton, *m_pCancelButton;
    std::string m_szSelection;
    
protected:
    virtual bool HandleEvent(CWidgetHandler *p, int type);

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;

    CMenuDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *info);

    void AddItem(const std::string &s, bool tags=false) { m_pMenu->AddItem(s, tags); };
    void AddItem(const char *s, bool tags=false) { m_pMenu->AddItem(s, tags); };
    void SetItem(const std::string &s) { m_pMenu->SetCurrent(s); };
    void SetItem(const char *s) { m_pMenu->SetCurrent(s); };
    
    const std::string &Run(void);
};

#endif
