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

#include "luaprogressbar.h"
#include "tui/progressbar.h"

// -------------------------------------
// Lua Progress Bar Class
// -------------------------------------

CLuaProgressBar::CLuaProgressBar(const char *desc) : CBaseLuaWidget(desc)
{
    StartPack(m_pProgressBar = new NNCurses::CProgressBar(0.0f, 100.0f), false, false, 0, 0);
}

void CLuaProgressBar::SetProgress(int n)
{
    m_pProgressBar->SetCurrent(static_cast<float>(n));
}

