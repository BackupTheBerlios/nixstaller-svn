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

#ifndef NCURS_H
#define NCURS_H

#include <vector>
#include <list>
#include <sstream>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <cdk/cdk.h>
#include "main.h"

#define NO_FILE -1
#define ESCAPE  -2

int ReadDir(const std::string &dir, char ***list);
int SwitchButtonK(EObjectType cdktype, void *object, void *clientData, chtype key);
int ScrollParamMenuK(EObjectType cdktype, void *object, void *clientData, chtype key);
int ViewFile(char *file, char **buttons, int buttoncount, char *title, bool showabout=true, bool showexit=true);
void WarningBox(const char *msg, ...);
bool YesNoBox(const char *msg, ...);
void ExtrThinkFunc(void *p);
void PrintExtrOutput(const char *msg, void *p);
void InstThinkFunc(void *p);
int ShowAboutK(EObjectType cdktype, void *object, void *clientData, chtype key);

inline int dummyK(EObjectType cdktype GCC_UNUSED, void *object, void *clientData GCC_UNUSED,
                  chtype key GCC_UNUSED) { return true; };

#include "widgets.h"

extern WINDOW *MainWin;
extern CDKSCREEN *CDKScreen;
extern CButtonBar ButtonBar;

inline void PrintInstOutput(const char *msg, void *p) { ((CCDKSWindow *)p)->AddText(msg, false); };
inline void SUUpdateProgress(int percent, void *p) { ((CCDKHistogram *)p)->SetValue(0, 100, percent); };
inline void SUUpdateText(const std::string &str, void *p) { ((CCDKSWindow *)p)->AddText(str, false); };
inline int GetDefaultHeight(void) { return getbegy(ButtonBar.GetLabel()->GetLabel()->win)-4; };
inline int GetDefaultWidth(void) { return getmaxx(MainWin)/1.25; };
inline int GetMaxHeight(int pref) { int y=GetDefaultHeight(); return (y<pref) ? y : pref; }
inline int GetMaxWidth(int pref) { int x=GetDefaultWidth(); return (x<pref) ? x : pref; }

#endif
