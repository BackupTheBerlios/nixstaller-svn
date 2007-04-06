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

#include "tui.h"
#include "inputfield.h"

namespace NNCurses {

// -------------------------------------
// Input Field Class
// -------------------------------------

CInputField::CInputField(const std::string &t, EInputType e) : m_Text(t), m_eInputType(e),
                                                               m_StartPos(0), m_CursorPos(0)
{
    SetFColors(COLOR_YELLOW, COLOR_RED);
    SetDFColors(COLOR_WHITE, COLOR_RED);
    SetMinWidth(1);
    SetMinHeight(1);
}

void CInputField::Move(int n, bool relative)
{
    TSTLStrSize len = m_Text.length();
    int w = Width()-1; // -1, because we want to scroll before char is at last available position
    
    if (relative)
    {
        if (n < 0)
        {
            TSTLStrSize nabs = abs(n);
            if (nabs > m_CursorPos)
            {
                TSTLStrSize rest = (nabs - m_CursorPos);
                
                if (rest > m_StartPos)
                    m_StartPos = 0;
                else
                    m_StartPos -= rest;
                
                m_CursorPos = 0;
            }
            else
                m_CursorPos -= nabs;
        }
        else
        {
            TSTLStrSize newpos = m_CursorPos + n;
            
            // Don't go past str length and prevent overflows
            if (((newpos+m_StartPos) > len) || (newpos < m_CursorPos))
                newpos = (len - m_StartPos);
            
            m_CursorPos = newpos;
        }
    }
    else
    {
        m_StartPos = 0;
        m_CursorPos = n;
    }

    if (m_CursorPos > static_cast<TSTLStrSize>(w))
    {
        m_StartPos += (m_CursorPos - w);
        
        TSTLStrSize max = len - w;
        if (m_StartPos > max)
            m_StartPos = max;
                
        m_CursorPos = w;
    }
    
    RequestQueuedDraw();
}

void CInputField::DoDraw()
{
    TSTLStrSize end = m_StartPos + Width(), length = m_Text.length();
    
    if (end > length)
        end = length;
    
    mvwaddstr(GetWin(), 0, 0, m_Text.substr(m_StartPos, end).c_str());
    
    if (Focused())
        TUI.LockCursor(GetWX(GetWin()) + SafeConvert<int>(m_CursorPos), GetWY(GetWin()));
}

bool CInputField::CoreHandleKey(chtype key)
{
    if (key == KEY_LEFT)
        Move(-1, true);
    else if (key == KEY_RIGHT)
        Move(1, true);
    else if (key == KEY_HOME)
        Move(0, false);
    else if (key == KEY_END)
        Move(m_Text.length(), false);
    else if (IsEnter(key))
        PushEvent(EVENT_CALLBACK);
    else
        return false;
    
    return true;
}

void CInputField::UpdateFocus()
{
    if (!Focused())
        TUI.UnLockCursor();
}

void CInputField::SetText(const std::string &t)
{
    m_Text = t;
    m_StartPos = m_CursorPos = 0;
    RequestQueuedDraw();
}

}
