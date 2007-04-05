/*
    Copyright (C) 2007 Rick Helmus (rhelmus_AT_gmail.com)

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

#ifndef MENUWINDOW
#define MENUWINDOW

#include <string>
#include "main.h"
#include "basescroll.h"

namespace NNCurses {

class CMenu: public CBaseScroll
{
    struct SEntry
    {
        std::string id, name;
        bool operator <(const SEntry &s) { return name < s.name; }
        bool operator <(const std::string &s) { return name < s; }
        bool operator <(char c) { return name[0] < c; }
        SEntry(const std::string &i, const std::string &n) : id(i), name(n) { }
    };

    typedef std::vector<SEntry> TMenuList;

    TMenuList m_MenuList;
    std::string m_MarkedLine;
    int m_iMaxWidth, m_iMaxHeight;
    TSTLStrSize m_LongestLine;
    int m_iXOffset, m_iYOffset;
    int m_iCursorLine;

    int ScrollFieldWidth(void) const { return Width()-2; }
    int ScrollFieldHeight(void) const { return Height()-2; }
    void Move(int n);
    int GetCurrent(void) { return m_iYOffset + m_iCursorLine; }
    
protected:
    virtual void DoDraw(void);
    virtual int CoreRequestWidth(void);
    virtual int CoreRequestHeight(void);
    virtual bool CoreHandleKey(chtype key);
    virtual bool CoreCanFocus(void) { return true; }
    virtual void CoreScroll(int vscroll, int hscroll);
    virtual TScrollRange CoreGetRange(void);
    virtual TScrollRange CoreGetScrollRegion(void);

public:
    CMenu(int maxw, int maxh) : m_iMaxWidth(maxw), m_iMaxHeight(maxh), m_LongestLine(0),
                                m_iXOffset(0), m_iYOffset(0), m_iCursorLine(0) { }

    void AddEntry(const std::string &id, const std::string &name);
    void Select(const std::string &id) { m_MarkedLine = id; RequestQueuedDraw(); }
};


}


#endif
