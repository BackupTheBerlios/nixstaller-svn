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

#ifndef LABEL_H
#define LABEL_H

#include <string>
#include <vector>
#include "widget.h"

namespace NNCurses {

class CLabel: public CWidget
{
    typedef std::vector<std::string> TLinesList;
    
    bool m_bCenter;
    std::string m_szText;
    TLinesList m_Lines;
    
    void UpdateLines(void);
    
protected:
    virtual void CoreInit(void) { UpdateLines(); }
    virtual int CoreRequestWidth(void);
    virtual int CoreRequestHeight(void);
    virtual void DoDraw(void);
    
public:
    CLabel(const std::string &t) : m_bCenter(true) { SetText(t); }
    
    void Center(bool c) { m_bCenter = c; }
    void SetText(const std::string &t);
};


}

#endif
