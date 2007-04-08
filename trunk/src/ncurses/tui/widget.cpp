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

#include <assert.h>
#include "main.h"
#include "tui.h"
#include "group.h"

namespace NNCurses {

// -------------------------------------
// Base Widget Class
// -------------------------------------

CWidget::CWidget(void) : m_pParent(NULL), m_pParentWin(NULL), m_pNCursWin(NULL), m_iX(0), m_iY(0), m_iWidth(0),
                         m_iHeight(0), m_iMinWidth(0), m_iMinHeight(0), m_bInitialized(false), m_bBox(false),
                         m_bFocused(false), m_bEnabled(true), m_bSizeChanged(false), m_bColorsChanged(false),
                         m_bSetFColors(false), m_bSetDFColors(false)
{
}

CWidget::~CWidget()
{
    if (m_pParent)
        m_pParent->RemoveWidget(this);

    if (m_pNCursWin)
        DelWin(m_pNCursWin);
    
    // IMPORTANT: This needs to be at the end! It ensures this widget won't be in the queue, since this or
    // one of the derived destructors may call QueueDraw()
    TUI.RemoveFromQueue(this);
}

void CWidget::MoveWindow(int x, int y)
{
    int realx = x + GetWX(GetParentWin());
    int realy = y + GetWY(GetParentWin());
    
    MoveWin(this, realx, realy);
    MoveDerWin(this, x, y);
}

void CWidget::InitDraw()
{
    if (m_bColorsChanged)
    {
        if (m_bFocused)
            wbkgdset(m_pNCursWin, ' ' | TUI.GetColorPair(m_FColors.first, m_FColors.second) | A_BOLD);
        else
            wbkgdset(m_pNCursWin, ' ' | TUI.GetColorPair(m_DFColors.first, m_DFColors.second) | A_BOLD);
        
        m_bColorsChanged = false;
        
        UpdateColors();
    }
    
    if (m_bSizeChanged)
    {
        MoveWindow(0, 0); // Move to a safe position first
        WindowResize(this, Width(), Height());
        MoveWindow(X(), Y());
        UpdateSize();
        m_bSizeChanged = false;
    }

    WindowErase(this);

    if (HasBox())
        Border(this);
}

void CWidget::DrawWidget()
{
    InitDraw();
    DoDraw();
    RefreshWidget();
}

void CWidget::PushEvent(int type)
{
    CWidget *parent = m_pParent;
    
    while (parent && !parent->HandleEvent(this, type))
        parent = parent->m_pParent;
}

void CWidget::SetColorAttr(TColorPair colors, bool e)
{
    SetAttr(this, TUI.GetColorPair(colors.first, colors.second) | A_BOLD, e);
}

void CWidget::Draw()
{
    if (!m_bInitialized)
    {
        Init();
        m_bInitialized = true;
    }
    
    CoreDraw();
}

void CWidget::RequestQueuedDraw()
{
    TUI.QueueDraw(this);
    PushEvent(EVENT_REQQUEUEDDRAW);
}

void CWidget::Init()
{
    m_pNCursWin = derwin(GetParentWin(), Height(), Width(), Y(), X());
    
    if (!m_pNCursWin)
        throw Exceptions::CExFrontend("Could not create derived window");
    
    m_bSizeChanged = false;
    
    leaveok(m_pNCursWin, false);
    KeyPad(m_pNCursWin, true);
    Meta(m_pNCursWin, true);
    
    if (m_pParent)
    {
        if (!m_bSetFColors)
        {
            TColorPair colors = m_pParent->GetFColors();
            SetFColors(colors);
        }
        
        if (!m_bSetDFColors)
        {
            TColorPair colors = m_pParent->GetDFColors();
            SetDFColors(colors);
        }
    }
    
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

void CWidget::SetFColors(TColorPair colors)
{
    m_FColors = colors;
    m_bColorsChanged = m_bSetFColors = true;
}

void CWidget::SetDFColors(TColorPair colors)
{
    m_DFColors = colors;
    m_bColorsChanged = m_bSetDFColors = true;
}

WINDOW *CWidget::GetParentWin()
{
    return (m_pParent) ? m_pParent->GetWin() : m_pParentWin;
}


}
