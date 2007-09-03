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

#ifndef FLTK_LUACFGMENU_H
#define FLTK_LUACFGMENU_H

#include "main/install/luacfgmenu.h"
#include "luawidget.h"

class Fl_Button;
class Fl_Choice;
class Fl_File_Input;
class Fl_Hold_Browser;
class Fl_Input;

class CLuaCFGMenu: public CBaseLuaCFGMenu, public CLuaWidget
{
    Fl_Hold_Browser *m_pVarListView;
    Fl_Input *m_pInputField;
    Fl_Choice *m_pChoiceMenu;
    Fl_File_Input *m_pDirInput;
    Fl_Button *m_pBrowseButton;
    int m_ColumnWidths[2];
    
    virtual void CoreAddVar(const char *name);
    virtual void CoreUpdateLanguage(void);
    virtual void CoreGetHeight(int maxw, int maxh, int &outh);
    
    int GroupHeight(void) const { return 175; }
    int DirChooserSpacing(void) const { return 10; }
    
    void SetVarColumnW(const char *var);
    const char *ColumnText(const char *var);
    const char *CurSelection(void);
    void UpdateDirChooser(int w);
    void UpdateSelection(void);
    const char *AskPassword(LIBSU::CLibSU &suhandler);
    
public:
    CLuaCFGMenu(const char *desc);
    
    static void SelectionCB(Fl_Widget *w, void *p) { (static_cast<CLuaCFGMenu *>(p))->UpdateSelection(); }
    static void InputChangedCB(Fl_Widget *w, void *p);
    static void ChoiceChangedCB(Fl_Widget *w, void *p);
    static void BrowseCB(Fl_Widget *w, void *p);
};

#endif
