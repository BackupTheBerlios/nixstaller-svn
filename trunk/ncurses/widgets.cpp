#include "ncurs.h"

// CDK Label Wrapper

CCDKLabel::CCDKLabel(CDKSCREEN *pScreen, int x, int y, char **msg, int count, bool box,
                                   bool shadow) : CBaseCDKWidget(box), m_iCount(count)
{
    m_szLabelTxt = new char*[m_iCount];
    for (int i=0;i<m_iCount;i++) m_szLabelTxt[i] = strdup(msg[i]);
    m_pLabel = newCDKLabel(pScreen, x, y, m_szLabelTxt, m_iCount, box, shadow);
    if (!m_pLabel) throwerror(false, "Could not create text label");
}

CCDKLabel::CCDKLabel(CDKSCREEN *pScreen, int x, int y, const std::string &msg, bool box,
                     bool shadow) : CBaseCDKWidget(box), m_iCount(1)
{
    m_szLabelTxt = new char*[1];
    m_szLabelTxt[0] = strdup(msg.c_str());
    m_pLabel = newCDKLabel(pScreen, x, y, m_szLabelTxt, m_iCount, box, shadow);
    if (!m_pLabel) throwerror(false, "Could not create text label");
}

CCDKLabel::CCDKLabel(CDKSCREEN *pScreen, int x, int y, const char *msg, bool box,
                     bool shadow) : CBaseCDKWidget(box), m_iCount(1)
{
    m_szLabelTxt = new char*[1];
    m_szLabelTxt[0] = strdup(msg);
    m_pLabel = newCDKLabel(pScreen, x, y, m_szLabelTxt, m_iCount, box, shadow);
    if (!m_pLabel) throwerror(false, "Could not create text label");
}

void CCDKLabel::Destroy(void)
{
    if (!m_pLabel) return;

    CBaseCDKWidget::Destroy();
    destroyCDKLabel(m_pLabel);
    for (int i=0;i<m_iCount;i++) free(m_szLabelTxt[i]);
    delete [] m_szLabelTxt;
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

void CCDKDialog::Destroy()
{
    if (!m_pDialog) return;

    CBaseCDKWidget::Destroy();
    destroyCDKDialog(m_pDialog);
    m_pDialog = NULL;
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

void CCDKSWindow::AddText(char *txt, bool wrap, int pos)
{
    if (wrap)
    {
        std::istringstream istr(txt);
        std::string line, tmpstr;

        istr >> line; // Need atleast one word...
        while(istr >> tmpstr)
        {
            if ((line.length() + tmpstr.length() + 1) > (m_pSWindow->boxWidth-2))
            {
                addCDKSwindow(m_pSWindow, CreateText(line.c_str()), BOTTOM);
                line = tmpstr;
            }
            else
                line += " " + tmpstr;
        }
        addCDKSwindow(m_pSWindow, CreateText(line.c_str()), pos);
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
