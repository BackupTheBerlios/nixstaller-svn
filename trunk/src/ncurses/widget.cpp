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

#include "ncurses.h"
#include "widget.h"
#include "tui.h"

namespace NNCurses {

// -------------------------------------
// Base Widget Class
// -------------------------------------

CWidget::CWidget(void) : m_pParent(NULL), m_pParentWin(NULL), m_pNCursWin(NULL), m_iX(0), m_iY(0), m_iWidth(0),
                         m_iHeight(0), m_bBox(false), m_bFocused(false), m_bColorsChanged(false), m_bSizeChanged(false)
{
}

CWidget::~CWidget()
{
    delwin(m_pNCursWin);
}

void CWidget::MoveWin(int x, int y)
{
    x += GetWX(m_pParentWin);
    y += GetWY(m_pParentWin);
    
    int ret = mvwin(m_pNCursWin, y, x);
    
    if (ret != ERR)
    {
        // Child widgets won't move without this...
        if (m_pParent)
        {
            m_pNCursWin->_begx = x;
            m_pNCursWin->_begy = y;
        }
    }
}

void CWidget::Box()
{
    wborder(m_pNCursWin, 0, 0, 0, 0, 0, 0, 0, 0);
}

void CWidget::RefreshColors()
{
    if (m_bColorsChanged)
    {
        if (m_bFocused)
            wbkgdset(m_pNCursWin, ' ' | TUI.GetColorPair(m_FColors.first, m_FColors.second) | A_BOLD);
        else
            wbkgdset(m_pNCursWin, ' ' | TUI.GetColorPair(m_DFColors.first, m_DFColors.second) | A_BOLD);
        
        m_bColorsChanged = false;
    }
}

void CWidget::RefreshSize()
{
    if (m_bSizeChanged)
    {
        ::wresize(m_pNCursWin, Height(), Width());
        MoveWin(X(), Y());

/*        if (!m_pParent)
        {
            werase(m_pParentWin);
            wrefresh(m_pParentWin);
        }
        else
            m_pParent->Draw();*/
        
        m_bSizeChanged = false;
    }
}

void CWidget::RefreshWidget()
{
    werase(m_pNCursWin);

    if (m_bBox)
        Box();

    wrefresh(m_pNCursWin);
}

void CWidget::DrawWidget()
{
    RefreshColors();
    RefreshSize();
    RefreshWidget();
}

void CWidget::Init()
{
    m_pNCursWin = derwin(m_pParentWin, Height(), Width(), Y(), X());
    
    // UNDONE: Exception
    
    leaveok(m_pNCursWin, 0);
    keypad(m_pNCursWin, 1);
    meta(m_pNCursWin, 1);
    
    CoreInit();
}

void CWidget::SetSize(int x, int y, int w, int h)
{
    m_iX = x;
    m_iY = y;
    m_iWidth = w;
    m_iHeight = h;
    m_bSizeChanged = true;
}

// void CWidget::Resize(int x, int y, int w, int h)
// {
//     SetSize(x, y, w, h);
//     m_bSizeChanged = true;
//     
//     ::wresize(m_pNCursWin, h, w);
//     MoveWin(x, y);
//     
//     if (!m_pParent)
//     {
//         werase(m_pParentWin);
//         wrefresh(m_pParentWin);
//     }
//     else
//         m_pParent->Draw();
// }

void CWidget::SetFColors(int fg, int bg)
{
    m_FColors = TColorPair(fg, bg);
    m_bColorsChanged = true;
}

void CWidget::SetDFColors(int fg, int bg)
{
    m_DFColors = TColorPair(fg, bg);
    m_bColorsChanged = true;
}

// -------------------------------------
// Utils
// -------------------------------------

void Position(CWidget *widget, int x, int y)
{
    widget->SetSize(x, y, widget->Width(), widget->Height());
}

void Size(CWidget *widget, int w, int h)
{
    widget->SetSize(widget->X(), widget->Y(), w, h);
}

}
