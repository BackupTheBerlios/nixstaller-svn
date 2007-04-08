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

#include "main.h"
#include "tui.h"
#include "widget.h"
#include "group.h"

namespace {

void Check(int ret, const char *function)
{
    if (ret == ERR)
        throw Exceptions::CExFrontend(CreateText("ncurses function \'%s\' returned an error.", function));
}

}


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

void AddCh(CWidget *widget, int x, int y, chtype ch)
{
    /*Check*/(mvwaddch(widget->GetWin(), y, x, ch), "mvwaddch");
}

void AddStr(CWidget *widget, int x, int y, const char *str)
{
    /*Check*/(mvwaddstr(widget->GetWin(), y, x, str), "mvwaddstr");
}

void SetAttr(CWidget *widget, chtype attr, bool e)
{
    if (e)
        Check(wattron(widget->GetWin(), attr), "wattron");
    else
        Check(wattroff(widget->GetWin(), attr), "wattroff");
}

void MoveWin(CWidget *widget, int x, int y)
{
    Check(mvwin(widget->GetWin(), y, x), "mvwin");
}

void MoveDerWin(CWidget *widget, int x, int y)
{
    Check(mvderwin(widget->GetWin(), y, x), "mvwin");
}

void Border(CWidget *widget)
{
    Check(wborder(widget->GetWin(), 0, 0, 0, 0, 0, 0, 0, 0), "wborder");
}

void EndWin()
{
    /*Check*/(endwin(), "endwin");
}

void NoEcho()
{
    Check(noecho(), "noecho");
}

void CBreak()
{
    Check(cbreak(), "cbreak");
}

void KeyPad(WINDOW *win, bool on)
{
    Check(keypad(win, on), "keypad");
}

void Meta(WINDOW *win, bool on)
{
    Check(meta(win, on), "meta");
}

void StartColor()
{
    Check(start_color(), "start_color");
}

void InitPair(short pair, short f, short b)
{
    Check(init_pair(pair, f, b), "init_pair");
}

void Move(int x, int y)
{
    Check(move(y, x), "move");
}

void DelWin(WINDOW *win)
{
    Check(delwin(win), "delwin");
}

void WindowResize(CWidget *widget, int w, int h)
{
    Check(wresize(widget->GetWin(), h, w), "wresize");
}

void WindowErase(CWidget *widget)
{
    Check(werase(widget->GetWin()), "werase");
}

void Refresh(CWidget *widget)
{
    Check(wrefresh(widget->GetWin()), "wrefresh");
}

void HLine(CWidget *widget, int x, int y, chtype ch, int count)
{
    Check(mvwhline(widget->GetWin(), y, x, ch, count), "mvwhline");
}

}
