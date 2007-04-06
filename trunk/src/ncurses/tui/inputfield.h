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

#ifndef INPUTFIELD
#define INPUTFIELD

#include "main.h"
#include "widget.h"

namespace NNCurses {

class CInputField: public CWidget
{
public:
    enum EInputType { STRING, NUMERIC, FLOAT };
    
    std::string m_Text;
    EInputType m_eInputType;
    TSTLStrSize m_StartPos, m_CursorPos;
    
    void Move(int n, bool relative);
    
protected:
    virtual void DoDraw(void);
    virtual bool CoreHandleKey(chtype key);
    virtual void UpdateFocus(void);
    virtual bool CoreCanFocus(void) { return true; }
//     virtual void CoreSetCursorPos(void) { wmove(GetWin(), 0, m_CursorPos); }
    
public:
    CInputField(const std::string &t, EInputType e);
    
    void SetText(const std::string &t);
};


}

#endif
