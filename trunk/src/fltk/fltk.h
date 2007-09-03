/*
    Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)

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

#ifndef FLTK_H
#define FLTK_H

#include "main/main.h"

class Fl_Button;
class Fl_Pack;
class Fl_Return_Button;
class Fl_Text_Display;
class Fl_Widget;
class Fl_Window;

class CFLTKBase: virtual public CMain
{
    Fl_Text_Display *m_pAboutDisp;
    Fl_Return_Button *m_pAboutOKButton;
    Fl_Window *m_pAboutWindow;
    
    void CreateAbout(void);
    
protected:
    virtual char *GetPassword(const char *str);
    virtual void MsgBox(const char *str, ...);
    virtual bool YesNoBox(const char *str, ...);
    virtual int ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...);
    virtual void WarnBox(const char *str, ...);

    void ShowAbout(bool show);
    
public:
    CFLTKBase(void);
    virtual ~CFLTKBase(void);

    virtual void UpdateLanguage(void);
    
    void Run(void);
    
    static void AboutOKCB(Fl_Widget *, void *p) { ((CFLTKBase *)p)->ShowAbout(false); };
    static void ShowAboutCB(Fl_Widget *, void *p) { ((CFLTKBase *)p)->ShowAbout(true); };
};

// Utils
void SetButtonWidth(Fl_Button *button);
inline int ButtonHeight(void) { return 25; }
int PackSpacing(Fl_Pack *pack);

#endif
