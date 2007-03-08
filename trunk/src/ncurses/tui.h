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

#include "group.h"

namespace NNCurses {

class CTUI
{
    typedef std::map<int, std::map<int, int> > TColorMap;
    
    TColorMap m_ColorPairs;
    int m_iCurColorPair;
    std::vector<CGroup *> m_RootGroups;
    
public:
    CTUI(void) : m_iCurColorPair(0) { };
    
    void InitNCurses(void);
    void StopNCurses(void);
    void Run(int delay=5);
    void AddGroup(CGroup *g);
    
    WINDOW *GetRootWin(void) { return stdscr; }
    
    int GetColorPair(int fg, int bg);
};

extern CTUI TUI;

// Utils
int GetWX(WINDOW *w);
int GetWY(WINDOW *w);

}

#endif
