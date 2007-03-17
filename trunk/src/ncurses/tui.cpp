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
}

void CTUI::StopNCurses()
{
    while (!m_RootGroups.empty())
    {
        delete m_RootGroups.back();
        m_RootGroups.pop_back();
    }
        
    ::endwin();
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
    
    if (!m_QueuedDrawWidgets.empty())
    {
        for (std::vector<CWidget *>::iterator it=m_QueuedDrawWidgets.begin(); it!=m_QueuedDrawWidgets.end(); it++)
        {
            (*it)->Draw();
        }
        m_QueuedDrawWidgets.clear();
    }
    
    return true;
}

void CTUI::AddGroup(CGroup *g)
{
    m_RootGroups.push_back(g);
    g->SetParent(GetRootWin());
    g->Init();
    
    if (!m_pActiveGroup && g->CanFocus())
    {
        g->Focus(true);
        m_pActiveGroup = g;
    }
    
    g->SetSize(0, 0, g->RequestWidth(), g->RequestHeight());
}

void CTUI::ActivateGroup(CGroup *g)
{
    assert(std::find(m_RootGroups.begin(), m_RootGroups.end(), g) != m_RootGroups.end());
    
    if (m_pActiveGroup)
    {
        m_pActiveGroup->Focus(false);
        m_pActiveGroup->Draw();
    }
    
    g->Focus(true);
    g->Draw();
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
        for (std::vector<CWidget *>::iterator it=m_QueuedDrawWidgets.begin();
             it!=m_QueuedDrawWidgets.end(); it++)
        {
            if (IsParent(*it, w))
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

}
