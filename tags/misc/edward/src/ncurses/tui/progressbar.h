/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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

#ifndef PROGRESSBAR
#define PROGRESSBAR

#include "widget.h"

namespace NNCurses {

class CProgressBar: public CWidget
{
    float m_fMin, m_fMax, m_fCurrent;
    
    void EnableColors(bool e);
    
protected:
    virtual void DoDraw(void);
    
public:
    CProgressBar(float min, float max);
    
    void SetCurrent(float cur) { m_fCurrent = cur; RequestQueuedDraw(); }
    float Value(void) { return m_fCurrent; }
};


}

#endif
