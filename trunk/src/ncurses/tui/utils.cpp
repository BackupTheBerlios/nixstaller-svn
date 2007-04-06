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
#include "widget.h"
#include "group.h"

namespace NNCurses {

// -------------------------------------
// Utils
// -------------------------------------

int GetWX(WINDOW *w)
{
    int x, y;
    getbegyx(w, y, x);
    return x;
}

int GetWY(WINDOW *w)
{
    int x, y;
    getbegyx(w, y, x);
    return y;
}

int GetWWidth(WINDOW *w)
{
    int x, y;
    getmaxyx(w, y, x);
    return x-GetWX(w);
}

int GetWHeight(WINDOW *w)
{
    int x, y;
    getmaxyx(w, y, x);
    return y-GetWY(w);
}

bool IsParent(CWidget *parent, CWidget *child)
{
    CWidget *w = child->GetParentWidget();
    while (w)
    {
        if (w == parent)
            return true;
        w = w->GetParentWidget();
    }
    
    return false;
}

bool IsChild(CWidget *child, CWidget *parent)
{
    return IsParent(parent, child);
}

bool IsDirectChild(CWidget *child, CWidget *parent)
{
    return (child->GetParentWidget() == parent);
}

void EnableReverse(CWidget *widget, bool e)
{
    if (e)
        wattron(widget->GetWin(), A_REVERSE);
    else
        wattroff(widget->GetWin(), A_REVERSE);
}

}
