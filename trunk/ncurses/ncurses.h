/*
    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.

    Nixstaller is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Nixstaller is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nixstaller; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
    conditions of the GNU General Public License cover the whole combination.

    In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
    software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
    DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
    such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
    you include the source code of that other code when and as the GNU GPL requires distribution of source code.

    Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
    versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
    version without this exception; this exception also makes it possible to release a modified version which carries forward
    this exception.
*/

#ifndef NCURSES_H
#define NCURSES_H

#include "cursesapp.h"
#include "cursesm.h"
#include "cursesf.h"

#include "main.h"
#include "widgets.h"

#define CTRL(x)             ((x) & 0x1f)

class CNCursScreen: public NCursesApplication
{
protected:
    // Overide some functions from ncurses++
    int titlesize() const { return 1; };
    void title();
    //Soft_Label_Key_Set::Label_Layout useSLKs() const { return Soft_Label_Key_Set::PC_Style_With_Index; };
    void init_labels(Soft_Label_Key_Set& S) const;
    void handleArgs(int argc, char* argv[]);
    
public:
    CNCursScreen() : NCursesApplication(true) { };

    int run();
};

class CNCursBase: public virtual CMain
{
    NCursesPanel *m_pDummyPanel; // Used for updating screen
    
protected:
    virtual char *GetPassword(const char *str) { };
    virtual void MsgBox(const char *str, ...);
    virtual bool YesNoBox(const char *str, ...) { };
    virtual int ChoiceBox(const char *str, const char *button1, const char *button2, const char *button3, ...) { };
    virtual void Warn(const char *str, ...) { };
    
    virtual bool ReadConfig(void) { }; // UNDONE: Remove me!
    friend class CNCursScreen; // UNDONE: Remove me!
public:
    
    virtual ~CNCursBase(void) { delete m_pDummyPanel; };
    
    virtual bool Init(void) { m_pDummyPanel = new NCursesPanel; m_pDummyPanel->bkgd(' '|COLOR_PAIR(7)); };
};


#ifndef RELEASE
inline void debugline(const char *t, ...)
{ static char txt[1024]; va_list v; va_start(v, t); vsprintf(txt, t, v); va_end(v); mvprintw(4, 50, "DEBUG: %s", txt); ::refresh(); };
#endif

#endif
