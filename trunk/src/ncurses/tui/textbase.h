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
    int m_iMaxReqWidth, m_iMaxReqHeight;
    TLinesList m_Lines;
    std::string m_QueuedText;
    TSTLStrSize m_LongestLine;
    int m_iCurWidth;
    
    void UpdateText(int width);
    
protected:
    virtual void CoreDraw(void);
    virtual void DoDraw(void) = 0;
    virtual int CoreRequestWidth(void);
    virtual int CoreRequestHeight(void);
    virtual void UpdateSize(void);
    
    CTextBase(bool c, bool w);
    
    TLinesList &GetLineList(void) { return m_Lines; }
    TSTLStrSize GetLongestLine(void) { return m_LongestLine; }
    TLinesList::size_type LineCount(void) { return m_Lines.size(); }
    bool Center(void) { return m_bCenter; }
    bool Wrap(void) { return m_bWrap; }
    int MaxWidth(void) { return m_iMaxReqWidth; }
    int MaxHeight(void) { return m_iMaxReqHeight; }

public:
    void AddText(std::string t);
    void SetText(const std::string &t) { m_Lines.clear(); AddText(t); }
    void Clear(void);
    void SetMaxReqWidth(int w) { m_iMaxReqWidth = w; }
    void SetMaxReqHeight(int h) { m_iMaxReqHeight = h; }
};


}

#endif
