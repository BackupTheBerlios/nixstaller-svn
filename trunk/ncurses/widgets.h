#ifndef CDK_WIDGET_H
#define CDK_WIDGET_H

class CBaseCDKWidget
{
protected:
    short m_sBackColor;
    bool m_bBox;

public:
    CBaseCDKWidget(bool b) : m_bBox(b) { };
    virtual ~CBaseCDKWidget(void) { Destroy(); };

    virtual void Destroy(void) { UnSetBgColor(); };
    virtual void Draw(void) = NULL;
    virtual void SetBgColor(int color) = NULL;
    virtual void UnSetBgColor() { };
    virtual EExitType ExitType(void) { return vNORMAL; };
    virtual void Bind(chtype key, BINDFN function, void *data) = NULL;
};

class CCDKLabel: public CBaseCDKWidget
{
    CDKLABEL *m_pLabel;
    int m_iCount;
    char **m_szLabelTxt;

public:
    CCDKLabel(CDKSCREEN *pScreen, int x, int y, char **msg, int count, bool box, bool shadow=false);
    CCDKLabel(CDKSCREEN *pScreen, int x, int y, const std::string &msg, bool box, bool shadow=false);
    CCDKLabel(CDKSCREEN *pScreen, int x, int y, const char *msg, bool box, bool shadow=false);
    virtual ~CCDKLabel(void) { Destroy(); };

    virtual void Draw(void) { drawCDKLabel(m_pLabel, m_bBox); };
    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKLabelBackgroundColor(m_pLabel, CreateText("</B/%d>", color));
                                         m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKLabelBackgroundColor(m_pLabel, CreateText("<!%d!B>", m_sBackColor)); };
    virtual void Bind(chtype key, BINDFN function, void *data) { bindCDKObject(vLABEL, m_pLabel, key, function, data); };
    
    CDKLABEL *GetLabel(void) { return m_pLabel; };
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
    virtual void Bind(chtype key, BINDFN function, void *data) { bindCDKObject(vBUTTONBOX, m_pBBox, key, function, data); };
    
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
    virtual void Bind(chtype key, BINDFN function, void *data) { bindCDKObject(vSCROLL, m_pScroll, key, function, data); };
    
    int Activate(chtype *actions = NULL) { return activateCDKScroll(m_pScroll, actions); };
    CDKSCROLL *GetScroll(void) { return m_pScroll; };
    int GetCurrent(void) { return getCDKScrollCurrent(m_pScroll); };
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
    virtual void Bind(chtype key, BINDFN function, void *data) { bindCDKObject(vALPHALIST, m_pAList, key, function, data); };
    
    char *Activate(chtype *actions = NULL) { return activateCDKAlphalist(m_pAList, actions); };
    CDKALPHALIST *GetAList(void) { return m_pAList; };
    void SetContent(char **list, int count) { setCDKAlphalistContents(m_pAList, list, count); };
};

class CCharListHelper;

class CCDKDialog: public CBaseCDKWidget
{
    CDKDIALOG *m_pDialog;
    CCharListHelper m_CharList;

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
    virtual void Bind(chtype key, BINDFN function, void *data) { bindCDKObject(vDIALOG, m_pDialog, key, function, data); };
    
    int Activate(chtype *actions = NULL) { return activateCDKDialog(m_pDialog, actions); };
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
    virtual void Bind(chtype key, BINDFN function, void *data) { bindCDKObject(vSWINDOW, m_pSWindow, key, function, data); };
    
    void AddText(const std::string &txt, bool wrap=true, int pos=BOTTOM) { AddText(CreateText(txt.c_str()), wrap, pos); };
    void AddText(const char *txt, bool wrap=true, int pos=BOTTOM) { AddText(CreateText(txt), wrap, pos); };
    void AddText(char *str, bool wrap=true, int pos=BOTTOM);
    
    int Exec(const char *command, int pos=BOTTOM) { return Exec(CreateText(command), pos); };
    int Exec(const std::string &command, int pos=BOTTOM) { return Exec(CreateText(command.c_str()), pos); };
    int Exec(char *command, int pos=BOTTOM) { return execCDKSwindow(m_pSWindow, command, pos); };

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
    virtual void Bind(chtype key, BINDFN function, void *data) { bindCDKObject(vENTRY, m_pEntry, key, function, data); };
    
    void SetHiddenChar(chtype ch) { setCDKEntryHiddenChar(m_pEntry, ch); };
    
    void SetValue(std::string &str) { SetValue(const_cast<char*>(str.c_str())); };
    void SetValue(const char *str) { SetValue(const_cast<char*>(str)); };
    void SetValue(char *str) { setCDKEntryValue(m_pEntry, str); };
    
    char *Activate(chtype *actions = NULL) { return activateCDKEntry(m_pEntry, actions); };
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
    virtual void Bind(chtype key, BINDFN function, void *data) { bindCDKObject(vHISTOGRAM, m_pHistogram, key, function,
                                                                 data); };
    
    void SetHistogram(EHistogramDisplayType vtype, int statspos, int min, int max, int cur, chtype fillch,
                      chtype statattr=A_BOLD);
    void SetValue(int min, int max, int cur) { setCDKHistogramValue(m_pHistogram, min, max, cur); };
    CDKHISTOGRAM *GetHistogram(void) { return m_pHistogram; };
};

#endif
