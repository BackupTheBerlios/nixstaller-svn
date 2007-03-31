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

#ifndef TEXTBASE
#define TEXTBASE

#include "main.h"
#include "widget.h"

namespace NNCurses {

class CTextBase: public CWidget
{
protected:
    typedef std::vector<std::string> TLinesList;
    
private:
    bool m_bCenter, m_bWrap;
    int m_iMaxWidth, m_iMaxHeight;
    TLinesList m_Lines;
    std::string m_QueuedText;
    TSTLStrSize m_LongestLine;
    
    void UpdateText(void);
    
protected:
    virtual void CoreDraw(void);
    virtual void DoDraw(void) = 0;
    virtual int CoreRequestWidth(void);
    virtual int CoreRequestHeight(void);
    
    CTextBase(bool c, bool w);
    
    TLinesList &GetLineList(void) { return m_Lines; }
    TSTLStrSize GetLongestLine(void) { return m_LongestLine; }
    TLinesList::size_type LineCount(void) { return m_Lines.size(); }
    bool Center(void) { return m_bCenter; }
    bool Wrap(void) { return m_bWrap; }
    int MaxWidth(void) { return m_iMaxWidth; }
    int MaxHeight(void) { return m_iMaxHeight; }

public:
    void AddText(const std::string &t) { m_QueuedText += t; }
    void SetText(const std::string &t) { m_Lines.clear(); AddText(t); }
    void SetMaxWidth(int w) { m_iMaxWidth = w; }
    void SetMaxHeight(int h) { m_iMaxHeight = h; }
};


}

#endif
