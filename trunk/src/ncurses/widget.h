/*
    Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.
    
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version. 
    
    This program is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE. See the GNU General Public License for more details. 
    
    You should have received a copy of the GNU General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
    St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef WIDGET_H
#define WIDGET_H

#include <utility>

namespace NNCurses {

typedef std::pair<int, int> TColorPair;

class CGroup;

class CWidget
{
    CWidget *m_pParent;
    WINDOW *m_pParentWin, *m_pNCursWin;
    int m_iX, m_iY, m_iWidth, m_iHeight;
    int m_iMinWidth, m_iMinHeight;
    
    bool m_bInitialized;
    bool m_bBox;
    bool m_bFocused;

    TColorPair m_FColors, m_DFColors; // Focused and DeFocused colors
    bool m_bColorsChanged, m_bSizeChanged;
    
    void MoveWin(int x, int y);
    void Box(void);

protected:
    enum { EVENT_CALLBACK, EVENT_DATACHANGED, EVENT_REQSIZECHANGE };
    
//     CWidget(void); ENABLE
    
    virtual void CoreDraw(void) { DrawWidget(); }
    virtual void DoDraw(void) { }
    virtual void CoreInit(void) { }
    
    WINDOW *GetWin(void) { return m_pNCursWin; }
    
    virtual void UpdateFocus(void) { }
    virtual bool CoreCanFocus(void) { return false; }
    
    virtual bool CoreHandleKey(chtype key) { return false; };
    virtual bool HandleEvent(CWidget *emitter, int event) { return false; };
    
    virtual int CoreRequestWidth(void) { return GetMinWidth(); }
    virtual int CoreRequestHeight(void) { return GetMinHeight(); }
    
    void InitDraw(void);
    void RefreshWidget(void) { wrefresh(m_pNCursWin); }
    void DrawWidget(void); // Calls above 3 functions. Default behaviour, called from CoreDraw()
    
    void PushEvent(int type);
    
public:
    CWidget(void);
    virtual ~CWidget(void);

    void Draw(void);
    void Init(void);
    void SetSize(int x, int y, int w, int h);
    
    void SetFColors(int fg, int bg);
    void SetDFColors(int fg, int bg);
    
    int X(void) const { return m_iX; }
    int Y(void) const { return m_iY; }
    int Width(void) const { return m_iWidth; }
    int Height(void) const { return m_iHeight; }
    int GetMinWidth(void) { return m_iMinWidth; }
    int GetMinHeight(void) { return m_iMinHeight; }
    void SetMinWidth(int w) { m_iMinWidth = w; }
    void SetMinHeight(int h) { m_iMinHeight = h; }
    int RequestWidth(void) { return CoreRequestWidth(); }
    int RequestHeight(void) { return CoreRequestHeight(); }
    
    void SetParent(CWidget *p) { m_pParent = p; m_pParentWin = NULL; }
    void SetParent(WINDOW *w) { m_pParent = NULL; m_pParentWin = w; }
    WINDOW *GetParentWin(void) { return (m_pParent) ? m_pParent->GetWin() : m_pParentWin; }
    
    bool HasBox(void) const { return m_bBox; }
    void SetBox(bool b) { m_bBox = b; }
    
    bool CanFocus(void) { return CoreCanFocus(); }
    bool Focused(void) { return m_bFocused; }
    void Focus(bool f) { m_bFocused = f; m_bColorsChanged = true; UpdateFocus(); }
    
    bool HandleKey(chtype key) { return CoreHandleKey(key); }
};

// Utils
void Position(CWidget *widget, int x, int y);
void Size(CWidget *widget, int w, int h);
bool HasSize(CWidget *widget);

}

#endif

