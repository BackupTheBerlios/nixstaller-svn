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

#ifndef TUI_H
#define TUI_H

#include <deque>

class CGroup;
class CBox;
class CWindowManager;
class CButtonBar;

namespace NNCurses {

class CTUI
{
    typedef std::map<int, std::map<int, int> > TColorMap;
    
    TColorMap m_ColorPairs;
    int m_iCurColorPair;
    CGroup *m_pActiveGroup;
    std::deque<CWidget *> m_QueuedDrawWidgets;
    CBox *m_pMainBox;
    CButtonBar *m_pButtonBar;
    CWindowManager *m_pWinManager;
    
public:
    CTUI(void) : m_iCurColorPair(0), m_pActiveGroup(NULL), m_pMainBox(NULL), m_pButtonBar(NULL), m_pWinManager(NULL) { };
    
    void InitNCurses(void);
    void StopNCurses(void);
    bool Run(int delay=5);
    void AddGroup(CGroup *g, bool activate);
    void ActivateGroup(CGroup *g);
    
    WINDOW *GetRootWin(void) { return stdscr; }
    
    int GetColorPair(int fg, int bg);
    void QueueDraw(CWidget *w);
    void RemoveFromQueue(CWidget *w);
};

extern CTUI TUI;

// Utils
int GetWX(WINDOW *w);
int GetWY(WINDOW *w);
int GetWWidth(WINDOW *w);
int GetWHeight(WINDOW *w);

}

#endif
