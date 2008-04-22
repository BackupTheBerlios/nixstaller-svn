/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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

#ifndef BASECHOICE
#define BASECHOICE

#include <vector>
#include "widget.h"

namespace NNCurses {

class CBaseChoice: public CWidget
{
protected:
    struct SEntry
    {
        std::string name;
        bool enabled;
        bool operator ==(const SEntry &e) { return ((name == e.name) && (enabled == e.enabled)); }
        SEntry(const std::string &n, bool e) : name(n), enabled(e) { }
    };
    
    typedef std::vector<SEntry> TChoiceList;
    
private:
    TChoiceList m_ChoiceList;
    int m_iSelection;
    int m_iUsedWidth;
    
    void Move(int n);
    
protected:
    virtual void DoDraw(void);
    virtual int CoreRequestWidth(void);
    virtual int CoreRequestHeight(void);
    virtual bool CoreHandleKey(chtype key);
    virtual bool CoreCanFocus(void) { return true; }
    virtual void CoreGetButtonDescs(TButtonDescList &list);
    virtual std::string CoreGetText(const SEntry &entry) = 0;
    virtual void CoreSelect(SEntry &entry) = 0;
    
    CBaseChoice(void) : m_iSelection(0), m_iUsedWidth(0) { }
    
    TChoiceList &GetChoiceList(void) { return m_ChoiceList; }
    int Selection(void) const { return m_iSelection; }

public:
    void AddChoice(const std::string &c) { m_ChoiceList.push_back(SEntry(c, false)); }
    void Select(int n) { CoreSelect(m_ChoiceList.at(n)); }
    void SetName(int n, const std::string &name) { m_ChoiceList.at(n).name = name; RequestQueuedDraw(); }
};


}

#endif
