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

#ifndef BOX_H
#define BOX_H

#include "widget.h"
#include "group.h"

namespace NNCurses {

class CBox: public CGroup
{
public:
    enum EDirection { HORIZONTAL, VERTICAL };
    
private:
    struct SBoxEntry
    {
        bool expand, fill;
        int padding;
        SBoxEntry(void) : expand(true), fill(true), padding(0) { }
        SBoxEntry(bool e, bool f, int p) : expand(e), fill(f), padding(p) { }
    };
    
    bool m_bUpdateLayout;
    std::map<CWidget *, SBoxEntry> m_BoxEntries;
    EDirection m_eDirection;
    int m_iSpacing;
    
    void UpdateLayout(void);
    
protected:
    virtual int CoreRequestWidth(void);
    virtual int CoreRequestHeight(void);
    virtual void CoreDraw(void);
    virtual void CoreAddWidget(CWidget *w) { m_bUpdateLayout = true; InitChild(w); }
    
public:
    CBox(EDirection dir, int s=0) : m_bUpdateLayout(true), m_eDirection(dir), m_iSpacing(s) { }
    
    // CGroup needs 2 versions
    void StartPack(CGroup *g, bool e, bool f, int p);
    void StartPack(CWidget *g, bool e, bool f, int p);
};



}

#endif
