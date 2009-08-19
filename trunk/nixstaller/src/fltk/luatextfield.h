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

#ifndef FLTK_LUATEXTFIELD_H
#define FLTK_LUATEXTFIELD_H

#include "main/frontend/luatextfield.h"
#include "luawidget.h"

class Fl_Text_Buffer;
class Fl_Text_Display;

class CLuaTextField: public CBaseLuaTextField, public CLuaWidget
{
    Fl_Text_Buffer *m_pBuffer;
    Fl_Text_Display *m_pDisplay;
    bool m_bWrap;
    std::string m_Size, m_Buffer;
    
    virtual void Load(const char *file);
    virtual void AddText(const char *text);
    virtual void ClearText(void);
    virtual void UpdateFollow(void);
    virtual int CoreRequestWidth(void) { return 150; }
    virtual int CoreRequestHeight(int maxw) { return FieldHeight(); }
    virtual void UpdateSize(void);
    
    void DoFollow(void);
    int FieldHeight(void) const;
    
public:
    CLuaTextField(const char *desc, bool wrap, const char *size);
    virtual ~CLuaTextField(void);
    
    static void BufferDumper(void *data);
};

#endif
