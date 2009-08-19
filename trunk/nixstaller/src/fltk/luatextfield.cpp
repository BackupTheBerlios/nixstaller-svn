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

#include "main/main.h"
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
    m_pDisplay->textsize(13);
    // Use a monospace font, so we can easily calc the width for line wrapping
    m_pDisplay->textfont(FL_COURIER);
    
    GetGroup()->add(m_pDisplay);
    
    Fl::add_timeout(0.2, BufferDumper, this);
}

CLuaTextField::~CLuaTextField()
{
    Fl::remove_timeout(BufferDumper, this);
}

void CLuaTextField::Load(const char *file)
{
    m_pBuffer->loadfile(file);
    if (Follow())
        DoFollow();
}

void CLuaTextField::AddText(const char *text)
{
    m_Buffer += text;
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
        fl_font(m_pDisplay->textfont(), m_pDisplay->textsize());
        int wrapw = (m_pDisplay->w() / fl_width(' ')) - 5; // Substract a little for scrollbar
        m_pDisplay->wrap_mode(true, wrapw);
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

void CLuaTextField::BufferDumper(void *data)
{
    CLuaTextField *me = static_cast<CLuaTextField *>(data);
    
    if (!me->m_Buffer.empty())
    {
        me->m_pBuffer->append(me->m_Buffer.c_str());
    
        const int size = me->m_pBuffer->length();
        if (size > me->MaxSize())
        {
            const int end = me->ClearSize() + (size - me->MaxSize());
            me->m_pBuffer->remove(0, end);
        }
    
        if (me->Follow())
            me->DoFollow();

        me->m_Buffer.clear();
    }
    
    Fl::repeat_timeout(0.04, BufferDumper, data);
}
