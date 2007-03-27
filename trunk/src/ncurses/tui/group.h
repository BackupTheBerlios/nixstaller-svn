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

#ifndef GROUP_H
#define GROUP_H

#include <vector>
#include <map>
#include "widget.h"

namespace NNCurses {

class CGroup: public CWidget
{
protected:
    typedef std::vector<CWidget *> TChildList;
    
private:
    TChildList m_Childs;
    CWidget *m_pFocusedWidget;
    std::map<CWidget *, CGroup *> m_GroupMap; // Widgets that are also groups
    
    bool IsGroupWidget(CWidget *w) { return (m_GroupMap[w] != NULL); }
    CGroup *GetGroupWidget(CWidget *w) { return m_GroupMap[w]; }
    bool CanFocusChilds(CWidget *w);
    
protected:
    virtual void CoreDraw(void);
    virtual void UpdateSize(void);
    virtual void UpdateFocus(void);
    virtual bool CoreHandleKey(chtype key);
    virtual void CoreAddWidget(CWidget *w) { InitChild(w); };
    virtual void CoreRemoveWidget(CWidget *w) { }
    virtual void CoreDrawChilds(void);

    void InitChild(CWidget *w);
    TChildList &GetChildList(void) { return m_Childs; }
    void DrawChilds(void) { CoreDrawChilds(); }
    CWidget *GetFocusedWidget(void) { return m_pFocusedWidget; }
    
    CGroup(void) : m_pFocusedWidget(NULL) { }

public:
    virtual ~CGroup(void) { Clear(); };
    
    void AddWidget(CGroup *g);
    void AddWidget(CWidget *w) { CoreAddWidget(w); };
    void RemoveWidget(CWidget *w);
    void Clear(void);
    bool Empty(void) { return m_Childs.empty(); }
    
    void FocusWidget(CWidget *w);
    bool SetNextFocWidget(bool cont); // cont : Checks widget after current focused widget
    bool SetPrevFocWidget(bool cont);
    void SetValidWidget(CWidget *ignore);
};

}

#endif
