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

#ifndef DIRDIALOG
#define DIRDIALOG

namespace LIBSU {
class CLibSU;
}

class Fl_File_Chooser;
class Fl_Widget;

class CFLTKDirDialog
{
    Fl_File_Chooser *m_pDirChooser;
    
    const char *AskPassword(LIBSU::CLibSU &suhandler);
    
public:
    CFLTKDirDialog(const char *d, const char *p, int t, const char *title);
    ~CFLTKDirDialog(void);
    
    void Run(void);
    const char *Value(void);
    void Value(const char *d);
    
    static void MKDirCB(Fl_Widget *w, void *p);
};


#endif
