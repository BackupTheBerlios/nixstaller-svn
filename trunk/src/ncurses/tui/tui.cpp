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

#include "tui.h"
#include "widget.h"
#include "group.h"
#include "box.h"
#include "windowmanager.h"
#include "buttonbar.h"

namespace NNCurses {

CTUI TUI;

// -------------------------------------
// TUI (Text User Interface) Class
// -------------------------------------

void CTUI::InitNCurses()
{
    endwin();
    initscr();
    
    noecho();
    cbreak();
    curs_set(0); // Hide cursor when possible

    if (has_colors())
        start_color();
    
    m_pMainBox = new CBox(CBox::VERTICAL, false);
    m_pMainBox->SetParent(GetRootWin());
    m_pMainBox->Init();
    m_pMainBox->SetSize(0, 0, GetWWidth(GetRootWin()), GetWHeight(GetRootWin()));
    
    m_pButtonBar = new CButtonBar;
    
    // Focused colors are used for keys, defocused colors for descriptions
    m_pButtonBar->SetFColors(COLOR_YELLOW, COLOR_RED);
    m_pButtonBar->SetDFColors(COLOR_WHITE, COLOR_RED);
    
    for (short s=0; s<5; s++)
        m_pButtonBar->AddButton("ESC", "Quit");
    
    m_pMainBox->EndPack(m_pButtonBar, false, false, 0);
    
    m_pWinManager = new CWindowManager;
    m_pMainBox->AddWidget(m_pWinManager);
    
    m_pMainBox->RequestQueuedDraw();

}

void CTUI::StopNCurses()
{
    delete m_pMainBox;
    endwin();
}

bool CTUI::Run(int delay)
{
    timeout(delay);
    
    // Handle keys
    chtype key = getch();
    
    if (key != static_cast<chtype>(ERR)) // Input available?
    {
        if (m_pActiveGroup)
        {
            if (!m_pActiveGroup->HandleKey(key))
            {
                if (key == 9) // 9 is TAB
                {
                    if (!m_pActiveGroup->SetNextFocWidget(true))
                        m_pActiveGroup->SetNextFocWidget(false);
                }
                else if (key == CTRL('p'))
                {
                    if (!m_pActiveGroup->SetPrevFocWidget(true))
                        m_pActiveGroup->SetPrevFocWidget(false);
                }
            }
        }
        
        if (key == CTRL('[')) // Escape pressed
            return false;
    }
    
    while (!m_QueuedDrawWidgets.empty())
    {
        CWidget *w = m_QueuedDrawWidgets.front();
        m_QueuedDrawWidgets.pop_front();
        // Draw after widget has been removed from queue, this way the widget or childs from the
        // widget can queue themselves.
        w->Draw();
    }
    
    return true;
}

void CTUI::AddGroup(CGroup *g, bool activate)
{
    m_pWinManager->AddWidget(g);
    
    if (activate)
        ActivateGroup(g);
    else
    {
        g->Focus(false);
        g->RequestQueuedDraw();
    }
}

void CTUI::ActivateGroup(CGroup *g)
{
    if (m_pActiveGroup)
    {
        m_pActiveGroup->Focus(false);
        m_pActiveGroup->RequestQueuedDraw();
    }
    
    m_pWinManager->FocusWidget(g);
    g->SetNextFocWidget(false); // UNDONE: Needed?
    m_pActiveGroup = g;
}

int CTUI::GetColorPair(int fg, int bg)
{
    if (!has_colors())
    {
        // Non-monochrome color requested?
        if (((fg != COLOR_BLACK) && (fg != COLOR_WHITE)) ||
              ((bg != COLOR_BLACK) && (bg != COLOR_WHITE)))
            return GetColorPair(COLOR_WHITE, COLOR_BLACK);
    }
    
    TColorMap::iterator it = m_ColorPairs.find(fg);
    if (it != m_ColorPairs.end())
    {
        std::map<int, int>::iterator it2 = it->second.find(bg);
        if (it2 != it->second.end())
            return COLOR_PAIR(it2->second);
    }
    
    m_iCurColorPair++;
    init_pair(m_iCurColorPair, fg, bg);
    m_ColorPairs[fg][bg] = m_iCurColorPair;
    return COLOR_PAIR(m_iCurColorPair);
}

void CTUI::QueueDraw(CWidget *w)
{
    bool done = false;
    while (!done)
    {
        done = true;
        for (std::deque<CWidget *>::iterator it=m_QueuedDrawWidgets.begin();
             it!=m_QueuedDrawWidgets.end(); it++)
        {
            if (*it == w)
                return;
            else if (IsParent(*it, w))
                return; // Parent is being redrawn, which will draw all childs
            else if (IsChild(*it, w))
            {
                // Remove any childs
                m_QueuedDrawWidgets.erase(it);
                done = false;
                break;
            }
        }
    }
    
    m_QueuedDrawWidgets.push_back(w);
}

void CTUI::RemoveFromQueue(CWidget *w)
{
    std::deque<CWidget *>::iterator it=std::find(m_QueuedDrawWidgets.begin(), m_QueuedDrawWidgets.end(), w);
    if (it != m_QueuedDrawWidgets.end())
        m_QueuedDrawWidgets.erase(it);
}

// -------------------------------------
// Utils
// -------------------------------------

int GetWX(WINDOW *w)
{
    int x, y;
    getbegyx(w, y, x);
    return x;
}

int GetWY(WINDOW *w)
{
    int x, y;
    getbegyx(w, y, x);
    return y;
}

int GetWWidth(WINDOW *w)
{
    int x, y;
    getmaxyx(w, y, x);
    return x-GetWX(w);
}

int GetWHeight(WINDOW *w)
{
    int x, y;
    getmaxyx(w, y, x);
    return y-GetWY(w);
}

}
