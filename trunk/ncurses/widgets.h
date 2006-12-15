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

#include <set>

struct SButtonBarEntry
{
    const char *button, *desc;
    bool global;
    SButtonBarEntry(const char *b, const char *d, bool g) : button(b), desc(d), global(g) { };
};

class CWidgetWindow;

class CWidgetHandler
{
    bool m_bEnabled;
    bool m_bFocused, m_bCanFocus;
    CWidgetHandler *m_pBoundKeyWidget; // Widgets which will recieve key events from this widget
    
    friend class CWidgetManager;
    friend class CInstaller;

protected:
    CWidgetHandler *m_pOwner;
    bool m_bDeleteMe; // Delete it later, incase we are in a loop from ie Run()

    std::list<CWidgetWindow *> m_ChildList;
    std::list<CWidgetWindow *>::iterator m_FocusedChild, m_NeedFocChild;

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
                                                                m_FocusedChild(m_ChildList.end()), m_NeedFocChild(m_ChildList.end()) { };

public:
    enum { EVENT_CALLBACK, EVENT_DATACHANGED };

    virtual ~CWidgetHandler(void);
    
    // _AddChild will actually add the widget to the child list
    // AddChild is just a simple wrapper that will also return the added child
    virtual void _AddChild(CWidgetWindow *p);
    template <typename C> C AddChild(C p)
    {
        _AddChild(p);
        return p;
    }
    
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
    void Quit(void) { m_bQuit = true; };
    
    virtual void _AddChild(CWidgetWindow *p);
    virtual void RemoveChild(CWidgetWindow *p);
    virtual void ActivateChild(CWidgetWindow *p);
    
    bool Run(unsigned delay=5); // delay == millisec to wait for input
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
        TSTLStrSize count;
        int fgcolor, bgcolor;
        color_entry_s(TSTLStrSize cn, int fg, int bg) : count(cn), fgcolor(fg), bgcolor(bg) { };
    };

    typedef std::map<TSTLStrSize, color_entry_s *> TColorTagList; // Index is used for starting position
    typedef std::map<TSTLStrSize, TSTLStrSize> TRevTagList; // Index is the starting position, char count is the value
    typedef std::set<TSTLStrSize> TCenterTagList;

    std::vector<line_entry_s *> m_Lines;
    TColorTagList m_ColorTags;
    TRevTagList m_ReversedTags;
    TCenterTagList m_CenteredIndexes;

private:
    CWidgetWindow *m_pWindow;
    TSTLStrSize m_TextLength, m_LongestLine, m_Width;
    TSTLVecSize m_CurrentLine, m_MaxHeight;
    bool m_bWrap, m_bHandleTags;
    
    TColorTagList::iterator GetNextColorTag(TColorTagList::iterator cur, TSTLStrSize curpos);
    TRevTagList::iterator GetNextRevTag(TRevTagList::iterator cur, TSTLStrSize curpos);
    
public:
    CFormattedText(CWidgetWindow *w, const std::string &str, bool wrap, TSTLVecSize maxh=std::numeric_limits<TSTLVecSize>::max());
    virtual ~CFormattedText(void) { Clear(); };

    static TSTLVecSize CalcLines(const std::string &str, bool wrap, TSTLStrSize width);
    
    virtual void AddText(const std::string &str);
    void SetText(const std::string &str) { Clear(); AddText(str); };
    void Print(TSTLVecSize startline=0, TSTLStrSize startw=0, TSTLVecSize endline=std::numeric_limits<TSTLVecSize>::max(),
               TSTLStrSize endw=std::numeric_limits<TSTLStrSize>::max());
    TSTLStrSize GetLines(void);
    TSTLVecSize GetLongestLine(void) { return m_LongestLine; };
    const std::string &GetText(TSTLVecSize line) { return m_Lines[line]->text; };
    bool Empty(void) { return !GetLines(); };
    void Clear(void);
    
    void AddCenterTag(TSTLVecSize line);
    void AddRevTag(TSTLVecSize line, TSTLStrSize pos, TSTLStrSize c);
    void AddColorTag(TSTLVecSize line, TSTLStrSize pos, TSTLStrSize c, int fg, int bg);
    
    void DelCenterTag(TSTLVecSize line);
    void DelRevTag(TSTLVecSize line, TSTLStrSize pos);
    void DelColorTag(TSTLVecSize line, TSTLStrSize pos);
};

class CWidgetWindow: public CWidgetHandler, public NCursesWindow
{
public:
    typedef std::map<int, std::map<int, int> > ColorMapType;

private:
    bool m_bBox, m_bInitialized, m_bFocusing;
    short m_sCurColor; // Current color pair used in formatted text
    chtype m_cLLCorner, m_cLRCorner, m_cULCorner, m_cURCorner;
    chtype m_cFocusedColors, m_cDefocusedColors;
    std::list<SButtonBarEntry> m_Buttons; // Buttons for button bar

    friend class CWidgetHandler;
    friend class CWidgetManager;

    void PushBBar(void);
    void PopBBar(void);
    
    static ColorMapType m_ColorPairs; // Map for easy getting color pairs
    static int m_iCurColorPair;
    static int m_iCursorY, m_iCursorX;
    
protected:
    std::string m_szTitle;
    
    virtual void CreateInit(void) { m_bInitialized = true; };
    virtual void Focus(void);
    virtual void LeaveFocus(void);
    virtual void Draw(void) { };
    
    void AddButton(const char *button, const char *desc) { m_Buttons.push_back(SButtonBarEntry(button, desc, false)); };
    void AddGlobalButton(const char *button, const char *desc) { m_Buttons.push_back(SButtonBarEntry(button, desc, true)); };
    
    int Box(void) { return ::wborder(w, 0, 0, 0, 0, m_cULCorner, m_cURCorner, m_cLLCorner, m_cLRCorner); };
    
    static void SetCursorPos(int y, int x) { m_iCursorY = y; m_iCursorX = x; }; // Lock cursor position
    static void ReleaseCursor(void) { m_iCursorY = RawMaxY(); m_iCursorX = MaxX(); };
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
    virtual void CreateInit(void) { CWidgetWindow::CreateInit(); AddButton("ENTER", "Activate button"); };
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CButton(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
            const std::string &text, char absrel = 'a');
    
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
    
    void SetMinMax(float min, float max) { m_fMinVal = min; m_fMaxVal = max; };
    void SetCurrent(float cur) { m_fCurVal = cur; };
    float GetValue(void) { return m_fCurVal; };
    void Scroll(float n); // Scroll n steps. Negative n is up, positive down.
};

class CTextLabel: public CWidgetWindow
{
    int m_iMaxHeight;
    TSTLVecSize m_CurLines;
    CFormattedText m_FMText;
    
protected:
    virtual void Draw(void);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CTextLabel(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel = 'a');
    
    void AddText(std::string text);
    void SetText(const std::string &text) { m_FMText.Clear(); AddText(text); };
    void SetText(const char *text) { m_FMText.Clear(); AddText(text); };
    
    static TSTLVecSize CalcHeight(int ncols, const std::string &text) { return CFormattedText::CalcLines(text, true, ncols); }
    static TSTLVecSize CalcHeight(int ncols, const char *text) { return CFormattedText::CalcLines(text, true, ncols); }
};

class CTextWindow: public CWidgetWindow
{
    CScrollbar *m_pVScrollbar, *m_pHScrollbar;
    CFormattedText *m_pFMText;
    CWidgetWindow *m_pTextWin; // Window containing the actual text
    bool m_bWrap, m_bFollow;
    TSTLVecSize m_CurrentLine;

    void ScrollToBottom(void);
    void HScroll(int n);
    void VScroll(int n);
    
protected:
    
    virtual void CreateInit(void);
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CTextWindow(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, bool wrap, bool follow,
                char absrel = 'a', bool box=true);
    virtual ~CTextWindow(void) { delete m_pFMText; };
    
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
            TSTLVecSize lineoffset;
            line_entry_s *line;
            unsorted_tag_s(C c, TSTLVecSize l, line_entry_s *pl) : entry(c), lineoffset(l), line(pl) { };
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
        void HighLight(TSTLVecSize line, bool h);
        TSTLVecSize Search(TSTLVecSize min, TSTLVecSize max, const std::string &key);
        TSTLVecSize Search(TSTLVecSize min, TSTLVecSize max, char key);
    };
    
    std::map<std::string, void*> m_DataMap;
    bool m_bInitCursor; // Does the current line needs to be highlighted?
    TSTLVecSize m_CursorLine;
    TSTLVecSize m_StartEntry; // First entry to display(for scrolling)
    CScrollbar *m_pVScrollbar, *m_pHScrollbar;
    CWidgetWindow *m_pTextWin; // Window containing the actual text
    CMenuText *m_pMenuText;
    
    void HScroll(int n);
    void VScroll(int n);
    TSTLVecSize GetCurrent(void) { return m_StartEntry+m_CursorLine; };
    
protected:
    virtual void CreateInit(void);
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CMenu(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x, char absrel = 'a');
    virtual ~CMenu(void) { Clear(); delete m_pMenuText; };
    
    void AddItem(std::string s, void *udata=NULL, bool tags=false);
    void Clear(void);
    const std::string &GetCurrentItemName(void) { return m_pMenuText->GetText(GetCurrent()); };
    void *GetCurrentItemData(void) { return m_DataMap[GetCurrentItemName()]; }
    void SetCurrent(const std::string &str);
    void SetCurrent(const char *str) { SetCurrent(std::string(str)); };
    bool Empty(void) { return m_pMenuText->Empty(); };
};

class CInputField: public CWidgetWindow
{
public:
    enum EInputType { INPUT_STRING, INPUT_INT, INPUT_FLOAT };
    
private:
    chtype m_chOutChar;
    std::string m_szText;
    int m_iMaxChars;
    TSTLStrSize m_CursorPos, m_ScrollOffset;
    CFormattedText m_FMText;
    EInputType m_eInpType;
    
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
                int max=-1, chtype out=0, EInputType type=INPUT_STRING);
    
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
                 float min, float max, char absrel = 'a');
    
    void SetCurrent(float n) { m_fCurrent = n; };
};

class CCheckbox: public CWidgetWindow
{
    struct SEntry
    {
        std::string name;
        bool enabled;
        SEntry(const std::string &n) : name(n), enabled(false) { };
    };
    
    std::vector<SEntry> m_BoxList;
    TSTLVecSize m_SelectedButton;

protected:
    virtual void CreateInit(void) { CWidgetWindow::CreateInit(); AddButton("SPACE", "Enable/Disable"); };
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;

    CCheckbox(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
              char absrel = 'a');

    void Add(const char *text) { m_BoxList.push_back(SEntry(text)); };
    void Add(const std::string &text) { m_BoxList.push_back(SEntry(text)); };
    void EnableBox(TSTLVecSize n) { m_BoxList.at(n).enabled = true; };
    void DisableBox(TSTLVecSize n) { m_BoxList.at(n).enabled = false; };
    bool IsEnabled(TSTLVecSize n) { return (m_BoxList.at(n).enabled); };
    void SetText(TSTLVecSize n, const std::string &text) { m_BoxList.at(n).name = text; };
};

class CRadioButton: public CWidgetWindow
{
    TSTLVecSize m_CheckedButton;
    std::vector<std::string> m_ButtonList;
    TSTLVecSize m_SelectedButton;

protected:
    virtual void CreateInit(void) { CWidgetWindow::CreateInit(); AddButton("SPACE", "Enable/Disable"); };
    virtual bool HandleKey(chtype ch);
    virtual void Draw(void);

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;

    CRadioButton(CWidgetWindow *owner, int nlines, int ncols, int begin_y, int begin_x,
                char absrel = 'a');

    void Add(const char *text) { m_ButtonList.push_back(text); };
    void Add(const std::string &text) { m_ButtonList.push_back(text); };
    void EnableButton(TSTLVecSize n) { m_CheckedButton = n; };
    TSTLVecSize EnabledButton(void) { return m_CheckedButton; };
    void SetText(TSTLVecSize n, const std::string &text) { m_ButtonList.at(n) = text; };
};

class CButtonBar: public CWidgetWindow
{
    typedef std::list<SButtonBarEntry> TButtonList;
    
    CTextLabel *m_pButtonText;
    std::list<TButtonList> m_ButtonTexts;
    CWidgetManager *m_pWidgetManager;
    bool m_bDirty; // True when button text has to be set

protected:
    virtual void CreateInit(void);
    virtual void Draw(void);
    
public:
    static chtype m_cDefaultColors;
    
    CButtonBar(CWidgetManager *owner, int nlines, int ncols, int begin_y, int begin_x);
    
    void Clear(void) { m_pButtonText->SetText(""); };
    void Push(void);
    void AddButton(const char *button, const char *desc);
    void AddGlobalButton(const char *button, const char *desc); // This will always be displayed untill the bar is pop'ed
    void Pop(void);
};

class CWidgetBox: public CWidgetWindow
{
    std::string m_szInfo;
    
protected:
    bool m_bFinished, m_bCanceled;
    CWidgetManager *m_pWidgetManager;
    CTextLabel *m_pLabel; // Derived classes will need coords from this label, so protected

    virtual void CreateInit(void);
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

    virtual void CreateInit(void);
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CMessageBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *text);
    
    void Run(void);
};

class CWarningBox: public CMessageBox
{
protected:
    virtual void CreateInit(void);
    
public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;

    CWarningBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *text);
};

class CYesNoBox: public CWidgetBox
{
    CButton *m_pYesButton, *m_pNoButton;

protected:
    virtual void CreateInit(void);
    virtual bool HandleEvent(CWidgetHandler *p, int type);

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    
    CYesNoBox(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *text);

    bool Run(void);
};

class CChoiceBox: public CWidgetBox
{
    std::string m_szButtonTitles[3];
    CButton *m_pButtons[3];
    int m_iSelectedButton;
    
protected:
    virtual void CreateInit(void);
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
    int m_iMax;
    CInputField *m_pTextField;
    CButton *m_pOKButton, *m_pCancelButton;

protected:
    virtual void CreateInit(void);
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
    virtual void CreateInit(void);
    virtual bool HandleEvent(CWidgetHandler *p, int type);
    virtual bool HandleKey(chtype ch);
    
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
    virtual void CreateInit(void);
    virtual bool HandleEvent(CWidgetHandler *p, int type);

public:
    static chtype m_cDefaultFocusedColors, m_cDefaultDefocusedColors;
    static std::string m_szBlank;

    CMenuDialog(CWidgetManager *owner, int maxlines, int ncols, int begin_y, int begin_x, const char *info);

    void AddItem(const std::string &s, void *udata=NULL, bool tags=false) { m_pMenu->AddItem(s, udata, tags); };
    void AddItem(const char *s, void *udata=NULL, bool tags=false) { m_pMenu->AddItem(s, udata, tags); };
    void SetItem(const std::string &s) { m_pMenu->SetCurrent(s); };
    void SetItem(const char *s) { m_pMenu->SetCurrent(s); };
    
    void Run(void);
    
    const std::string &GetItem(void) { return (!m_bCanceled) ? m_pMenu->GetCurrentItemName() : m_szBlank; };
    void *GetData(void) { return (!m_bCanceled) ? m_pMenu->GetCurrentItemData() : NULL; };
};

#endif
