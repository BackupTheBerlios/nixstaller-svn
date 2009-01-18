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

#include <locale.h>
#include <ctype.h>
#include <wctype.h>
#include "utf8.h"
#include "tui.h"
#include "inputfield.h"

namespace NNCurses {

// -------------------------------------
// Input Field Class
// -------------------------------------

CInputField::CInputField(const std::string &t, EInputType e, int max,
                         char out) : m_Text(t), m_iMaxChars(max), m_cOut(out), m_eInputType(e),
                                     m_StartPos(0), m_CursorPos(0)
{
//     SetFColors(COLOR_YELLOW, COLOR_RED);
//     SetDFColors(COLOR_WHITE, COLOR_RED);
    SetMinHeight(3);
    SetBox(true);
}

CInputField::~CInputField(void)
{
    if (Focused())
        TUI.UnLockCursor();
}

TSTLStrSize CInputField::GetStrPosition()
{
    std::string::iterator it = m_Text.begin();
    utf8::advance(it, m_StartPos + m_CursorPos, m_Text.end());
    return std::distance(m_Text.begin(), it);
}

void CInputField::Move(int n, bool relative)
{
    const TSTLStrSize chars = utf8::distance(m_Text.begin(), m_Text.end());
    const TSTLStrSize maxwidth = SafeConvert<TSTLStrSize>(Width()-2); // -3 because we have a frame and to keep cursor inside it
    
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
            
            // Don't go past str width and prevent overflows
            if (((newpos+m_StartPos) > chars) || (newpos < m_CursorPos))
                newpos = (chars - m_StartPos);
            
            m_CursorPos = newpos;
        }
    }
    else
    {
        m_StartPos = 0;
        m_CursorPos = std::min(SafeConvert<TSTLStrSize>(n), chars);
    }

    std::string::iterator start = m_Text.begin();
    utf8::advance(start, m_StartPos, m_Text.end());
    
    bool pastend = ((m_StartPos+m_CursorPos) >= chars);
    std::string::iterator end;
    if (!pastend)
        utf8::advance(end = start, m_CursorPos+1, m_Text.end());
    else
        end = m_Text.end();
    
    const TSTLStrSize realmaxw = GetFitfromW(m_Text, start, maxwidth, false);
    if ((MBWidth(std::string(start, end)) + pastend) > realmaxw)
    {
        while (start != end)
        {
            m_StartPos++;
            utf8::next(start, m_Text.end());
            if ((MBWidth(std::string(start, end)) + pastend) <= realmaxw)
                break;
        }
        m_CursorPos = utf8::distance(start, end) - 1 + pastend;
    }
    
    RequestQueuedDraw();
}

bool CInputField::ValidChar(wchar_t *ch)
{
    if (IsTAB(*ch) || !iswprint(*ch))
        return false;
    
    if ((m_eInputType == INT) || (m_eInputType == FLOAT))
    {
        lconv *lc = localeconv();
        if (strchr(lc->decimal_point, ','))
        {
            if (*ch == '.')
                *ch = ',';
        }
        
        std::string legal = LegalNrTokens((m_eInputType == FLOAT), m_Text, GetStrPosition());
        char mb[MB_CUR_MAX];
        
        int ret = wctomb(mb, *ch);
        mb[ret] = 0;

        if (legal.find(mb) == std::string::npos)
            return false; // Illegal char
    }
    
    return true;
}

void CInputField::Addch(wchar_t ch)
{
    TSTLStrSize chars = utf8::distance(m_Text.begin(), m_Text.end());
    
    if (m_iMaxChars && (SafeConvert<int>(chars) >= m_iMaxChars))
        return;
    
    TSTLStrSize pos = GetStrPosition();
    
    char mb[MB_CUR_MAX];
    int ret = wctomb(mb, ch);
    mb[ret] = 0;
    
    if (pos >= chars)
        m_Text += mb;
    else
        m_Text.insert(pos, mb);
    
    Move(1, true);
    PushEvent(EVENT_DATACHANGED);
}

void CInputField::Delch(TSTLStrSize pos)
{
    if ((m_Text.empty()) || (pos >= SafeConvert<TSTLStrSize>(utf8::distance(m_Text.begin(), m_Text.end()))))
        return;
    
    std::string::iterator start = m_Text.begin(), end;
    utf8::advance(start, pos, m_Text.end());
    utf8::next(end = start, m_Text.end());
    m_Text.erase(start, end);
    RequestQueuedDraw();
    PushEvent(EVENT_DATACHANGED);
}

void CInputField::DoDraw()
{
    std::string::iterator first = m_Text.begin(), last;
    utf8::advance(first, m_StartPos, m_Text.end());
    last = first + GetMBLenFromW(std::string(first, m_Text.end()), Width()-2); // -2 for the border and cursor
    
    if (m_cOut)
    {
        TSTLStrSize charn = utf8::distance(first, last);
        AddStr(this, 1, 1, std::string(charn, m_cOut).c_str());
    }
    else
        AddStr(this, 1, 1, std::string(first, last).c_str());
    
    if (Focused())
    {
        std::string::iterator start = m_Text.begin();
        utf8::advance(start, m_StartPos, m_Text.end());
        int cursx = SafeConvert<int>(GetMBWidthFromC(m_Text, start, m_CursorPos)) + 1;
        TUI.LockCursor(GetWX(GetWin()) + cursx, GetWY(GetWin())+1);
    }
}

bool CInputField::CoreHandleKey(wchar_t key)
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
    else if (key == KEY_DC)
        Delch(m_StartPos + m_CursorPos);
    else if (IsBackspace(key))
    {
        Delch(m_StartPos + m_CursorPos - 1);
        Move(-1, true);
    }
    else
    {
        if (ValidChar(&key))
            Addch(key);
        else
            return false;
    }
    
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
//     m_StartPos = m_CursorPos = 0;
    Move(m_Text.length(), false);
    RequestQueuedDraw();
}

}
