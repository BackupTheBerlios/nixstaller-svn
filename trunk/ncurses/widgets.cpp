#include "ncurs.h"

// CDK Label Wrapper

CCDKLabel::CCDKLabel(CDKSCREEN *pScreen, int x, int y, char **msg, int count, bool box,
                                   bool shadow) : CBaseCDKWidget(box), m_iCount(count)
{
    m_szLabelTxt = new char*[m_iCount];
    for (int i=0;i<m_iCount;i++) m_szLabelTxt[i] = strdup(msg[i]);
    CreateLabel(pScreen, x, y, shadow);
}

CCDKLabel::CCDKLabel(CDKSCREEN *pScreen, int x, int y, const std::string &msg, bool box,
                                   bool shadow) : CBaseCDKWidget(box), m_iCount(1)
{
    m_szLabelTxt = new char*[1];
    m_szLabelTxt[0] = strdup(msg.c_str());
    CreateLabel(pScreen, x, y, shadow);
}

CCDKLabel::CCDKLabel(CDKSCREEN *pScreen, int x, int y, const char *msg, bool box,
                                   bool shadow) : CBaseCDKWidget(box), m_iCount(1)
{
    m_szLabelTxt = new char*[1];
    m_szLabelTxt[0] = strdup(msg);
    CreateLabel(pScreen, x, y, shadow);
}

void CCDKLabel::CreateLabel(CDKSCREEN *pScreen, int x, int y, bool shadow)
{
    m_pLabel = newCDKLabel(pScreen, x, y, m_szLabelTxt, m_iCount, m_bBox, shadow);
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

// CDK Scroll Wrapper

CCDKScroll::CCDKScroll(CDKSCREEN *pScreen, int x, int y, int h, int w, int sbpos, char *title, bool box,
                                    bool numbers, bool shadow) : CBaseCDKWidget(box), m_bHasItem(false)
{
    m_szDummyItem = new char*[2];
    m_szDummyItem[0] = "dummy item";
    m_szDummyItem[1] = NULL;
    m_pScroll = newCDKScroll(CDKScreen, x, y, sbpos, h, w, title, m_szDummyItem, 1, numbers, A_REVERSE, box, shadow);
}

void CCDKScroll::Destroy(void)
{
    if (!m_pScroll) return;

    CBaseCDKWidget::Destroy();
    destroyCDKScroll(m_pScroll);
    delete [] m_szDummyItem;
    m_pScroll= NULL;
}

void CCDKScroll::AddItem(char *str)
{
    if (!m_bHasItem)
    {
        m_bHasItem = true;
        deleteCDKScrollItem(m_pScroll, 0);
    }
    addCDKScrollItem(m_pScroll, str);
}
