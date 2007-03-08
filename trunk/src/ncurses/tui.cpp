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
    ::endwin();
}

void CTUI::Run(int delay)
{
    timeout(delay);
    
    for (short s=0;s<10;s++)
    {
        m_RootGroups[0]->SetPrevWidget();
        sleep(2);
    }
    
    // Handle keys
}

void CTUI::AddGroup(CGroup *g)
{
    m_RootGroups.push_back(g);
    g->SetParent(GetRootWin());
    g->Init();
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
