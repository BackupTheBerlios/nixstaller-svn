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

#include "luatextfield.h"
#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

// -------------------------------------
// Lua Text Field Class
// -------------------------------------

CLuaTextField::CLuaTextField(const char *desc, bool wrap, const char *size) : CBaseLuaWidget(desc), m_bWrap(wrap), m_Size(size)
{
    m_pBuffer = new Fl_Text_Buffer;
    
    m_pDisplay = new Fl_Text_Display(0, 0, 0, FieldHeight());
    m_pDisplay->buffer(m_pBuffer);
    m_pDisplay->textsize(12);
    
    GetGroup()->add(m_pDisplay);
}

void CLuaTextField::Load(const char *file)
{
    m_pBuffer->loadfile(file);
    if (Follow())
        DoFollow();
}

void CLuaTextField::AddText(const char *text)
{
    m_pBuffer->append(text);
    
    const int size = m_pBuffer->length();
    if (size > MaxSize())
    {
        const int end = ClearSize() + (size - MaxSize());
        m_pBuffer->remove(0, end);
    }
    
    if (Follow())
        DoFollow();
}

void CLuaTextField::ClearText()
{
    m_pBuffer->text("");
}

void CLuaTextField::UpdateFollow()
{
    DoFollow();
}

void CLuaTextField::UpdateSize()
{
    m_pDisplay->size(GetGroup()->w(), m_pDisplay->h());
    
    if (m_bWrap)
    {
        // Undocumented in FLTK: passing 0 will automaticly take the width margin for wrapping
        m_pDisplay->wrap_mode(true, 0);
    }
}

void CLuaTextField::DoFollow()
{
    // Move cursor to last line and last word
    m_pDisplay->scroll(m_pBuffer->length(), m_pDisplay->word_end(m_pBuffer->length()));
    Fl::wait(0.0); // Update screen
}

int CLuaTextField::FieldHeight(void) const
{
    if (m_Size == "small")
        return 120;
    else if (m_Size == "medium")
        return 170;
    else // big
        return 220;
}
