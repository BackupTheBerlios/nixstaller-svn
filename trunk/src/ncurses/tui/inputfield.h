/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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

#ifndef INPUTFIELD_H
#define INPUTFIELD_H

#include "main/main.h"
#include "widget.h"

namespace NNCurses {

class CInputField: public CWidget
{
public:
    enum EInputType { STRING, INT, FLOAT };
    
private:
    std::string m_Text;
    int m_iMaxChars;
    char m_cOut;
    EInputType m_eInputType;
    TSTLStrSize m_StartPos, m_CursorPos;
    
    TSTLStrSize GetStrPosition(void);
    void Move(int n, bool relative);
    bool ValidChar(chtype *ch);
    void Addch(chtype ch);
    void Delch(TSTLStrSize pos);
    
protected:
    virtual void DoDraw(void);
    virtual bool CoreHandleKey(chtype key);
    virtual void UpdateFocus(void);
    virtual bool CoreCanFocus(void) { return true; }
    
public:
    CInputField(const std::string &t, EInputType e, int max=0, char out=0);
    virtual ~CInputField(void);
    
    void SetText(const std::string &t);
    std::string Value(void) const { return m_Text; }
};


}

#endif
