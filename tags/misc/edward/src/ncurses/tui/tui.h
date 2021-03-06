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

#ifndef TUI_H
#define TUI_H

#include <deque>
#include <map>
#include "include/ncurses/curses.h"

namespace NNCurses {

class CWidget;
class CGroup;
class CBox;
class CWindowManager;
class CButtonBar;

class CTUI
{
    typedef std::map<int, std::map<int, int> > TColorMap;
    
    bool m_bQuit;
    TColorMap m_ColorPairs;
    int m_iCurColorPair;
    std::deque<CWidget *> m_QueuedDrawWidgets;
    CBox *m_pMainBox;
    CButtonBar *m_pButtonBar;
    CWindowManager *m_pWinManager;
    
    std::pair<int, int> m_CursorPos;
    
    void UpdateButtonBar(void);
    
    friend class CWindowManager;
    
public:
    CTUI(void) : m_bQuit(false), m_iCurColorPair(0), m_pMainBox(NULL), m_pButtonBar(NULL),
                 m_pWinManager(NULL) { };
    
    void InitNCurses(void);
    void StopNCurses(void);
    bool Run(int delay=5);
    void Quit(void) { m_bQuit = true; }
    void AddGroup(CGroup *g, bool activate);
    void ActivateGroup(CGroup *g);
    
    WINDOW *GetRootWin(void) { return stdscr; }
    
    int GetColorPair(int fg, int bg);
    void QueueDraw(CWidget *w);
    void RemoveFromQueue(CWidget *w);
    
    void QueueRefresh(void);
    
    void LockCursor(int x, int y) { m_CursorPos.first = x; m_CursorPos.second = y; }
    void UnLockCursor(void) { m_CursorPos.first = m_CursorPos.second = 0; }
};

extern CTUI TUI;

// Utils
int GetWX(WINDOW *w);
int GetWY(WINDOW *w);
int GetWWidth(WINDOW *w);
int GetWHeight(WINDOW *w);
inline int GetMaxWidth(void) { return GetWWidth(stdscr); }
inline int GetMaxHeight(void) { return GetWHeight(stdscr); }
inline bool IsEnter(chtype ch) { return ((ch==KEY_ENTER) || (ch=='\n') || (ch=='\r')); }
inline chtype CTRL(chtype ch) { return ((ch) & 0x1f); }
inline bool IsEscape(chtype ch) { return ch == CTRL('['); }
inline bool IsBackspace(chtype ch) { return ((ch == KEY_BACKSPACE) || (ch == 0x07f)); }
inline bool IsTAB(chtype ch) { return ch == 9; }
bool IsParent(CWidget *parent, CWidget *child);
bool IsChild(CWidget *child, CWidget *parent);
bool IsDirectChild(CWidget *child, CWidget *parent);
void AddCh(CWidget *widget, int x, int y, chtype ch);
void AddStr(CWidget *widget, int x, int y, const char *str);
void SetAttr(CWidget *widget, chtype attr, bool e);
void MoveWin(CWidget *widget, int x, int y);
void MoveDerWin(CWidget *widget, int x, int y);
void Border(CWidget *widget);
void EndWin(void);
void NoEcho(void);
void CBreak(void);
void KeyPad(WINDOW *widget, bool on);
void Meta(WINDOW *widget, bool on);
void StartColor(void);
void InitPair(short pair, short f, short b);
void Move(int x, int y);
void DelWin(WINDOW *win);
void WindowResize(CWidget *widget, int w, int h);
void WindowErase(CWidget *widget);
void Refresh(CWidget *widget);
void WNOUTRefresh(WINDOW *win);
void DoUpdate(void);
void HLine(CWidget *widget, int x, int y, chtype ch, int count);

}

#endif
