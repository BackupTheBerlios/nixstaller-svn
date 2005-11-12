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

    virtual void Activate(void) { };
    virtual void Destroy(void) { UnSetBgColor(); };
    virtual void Draw(void) { };
    virtual void SetBgColor(int color) { };
    virtual void UnSetBgColor() { };
    virtual EExitType ExitType(void) { return vNORMAL; };
};

class CCDKLabel: public CBaseCDKWidget
{
    CDKLABEL *m_pLabel;
    int m_iCount;
    char **m_szLabelTxt;

    void CreateLabel(CDKSCREEN *pScreen, int x, int y, bool shadow);

public:
    CCDKLabel(CDKSCREEN *pScreen, int x, int y, char **msg, int count, bool box, bool shadow=false);
    CCDKLabel(CDKSCREEN *pScreen, int x, int y, const std::string &msg, bool box, bool shadow=false);
    CCDKLabel(CDKSCREEN *pScreen, int x, int y, const char *msg, bool box, bool shadow=false);
    virtual ~CCDKLabel(void) { Destroy(); };

    virtual void Draw(void) { drawCDKLabel(m_pLabel, m_bBox); };
    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKLabelBackgroundColor(m_pLabel, CreateText("</B/%d>", color)); m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKLabelBackgroundColor(m_pLabel, CreateText("<!%d!B>", m_sBackColor)); };
};

class CCDKScroll: public CBaseCDKWidget
{
    char **m_szDummyItem;
    bool m_bHasItem;
    CDKSCROLL *m_pScroll;

public:
    CCDKScroll(CDKSCREEN *pScreen, int x, int y, int h, int w, int sbpos, char *title, bool box=true,
                     bool numbers=false, bool shadow=false);
    virtual ~CCDKScroll(void) { Destroy(); };

    virtual void Destroy(void);
    virtual void SetBgColor(int color) { setCDKScrollBackgroundColor(m_pScroll, CreateText("</B/%d>", color));
                                                                                                    m_sBackColor = color; };
    virtual void UnSetBgColor() { setCDKScrollBackgroundColor(m_pScroll, CreateText("<!%d!B>", m_sBackColor)); };
    virtual EExitType ExitType(void) { return m_pScroll->exitType; };

    void AddItem(const std::string &str) { AddItem(str.c_str()); };
    void AddItem(char *str);
    void AddItem(const char *str) { AddItem(const_cast<char *>(str)); };

    int Activate(chtype *actions = NULL) { return activateCDKScroll(m_pScroll, actions); };
};

#endif
