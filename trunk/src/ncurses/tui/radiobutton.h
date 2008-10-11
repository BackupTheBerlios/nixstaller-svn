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

#ifndef RADIOBUTTON
#define RADIOBUTTON

#include "main/main.h"
#include "basechoice.h"

namespace NNCurses {

class CRadioButton: public CBaseChoice
{
    SEntry *m_pActiveEntry;
    bool m_bInitSelect;
    
protected:
    virtual void CoreInit(void);
    virtual std::string CoreGetText(const SEntry &entry);
    virtual void CoreSelect(SEntry &entry);
    virtual void CoreDeleteChoice(SEntry &entry);
    virtual void CoreGetButtonDescs(TButtonDescList &list);
    
public:
    CRadioButton(void) : m_pActiveEntry(NULL), m_bInitSelect(true) { }
    std::string GetSelection(void) { return m_pActiveEntry->name; }
};

}


#endif
