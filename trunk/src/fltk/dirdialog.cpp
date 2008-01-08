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

#include "main/main.h"
#include "dirdialog.h"
#include <FL/Fl_File_Chooser.H>

// -------------------------------------
// FLTK Directory Dialog Util Class
// -------------------------------------

CFLTKDirDialog::CFLTKDirDialog(const char *d, const char *p, int t, const char *title)
{
    m_pDirChooser = new Fl_File_Chooser(d, p, t, title);
    m_pDirChooser->preview(false);
    m_pDirChooser->previewButton->hide();
    m_pDirChooser->newButton->tooltip(Fl_File_Chooser::new_directory_tooltip);
    m_pDirChooser->newButton->callback(MKDirCB, this); // Hook in our own nice directory creation function :)
}

CFLTKDirDialog::~CFLTKDirDialog()
{
    delete m_pDirChooser;
}

const char *CFLTKDirDialog::AskPassword(LIBSU::CLibSU &suhandler)
{
    const char *ret = NULL;
    
    while (true)
    {
        ret = fl_password(GetTranslation("Your account doesn't have permissions to "
                "create the directory.\nTo create it with the root "
                "(administrator) account, please enter the administrative password below."));

        if (!ret || !ret[0])
            break;

        if (!suhandler.TestSU(ret))
        {
            if (suhandler.GetError() == LIBSU::CLibSU::SU_ERROR_INCORRECTPASS)
                fl_alert(GetTranslation("Incorrect password given, please retype."));
            else
            {
                fl_alert(GetTranslation("Could not use su or sudo to gain root access."));
                break;
            }
        }
        else
            break;
    }
    
    return ret;
}

void CFLTKDirDialog::Run()
{
    m_pDirChooser->show();
    while(m_pDirChooser->visible())
        Fl::wait();
}

const char *CFLTKDirDialog::Value()
{
    return m_pDirChooser->value();
}

void CFLTKDirDialog::Value(const char *d)
{
    m_pDirChooser->value(d);
}

void CFLTKDirDialog::MKDirCB(Fl_Widget *w, void *p)
{
    CFLTKDirDialog *dialog = static_cast<CFLTKDirDialog *>(p);
    const char *newdir = fl_input(GetTranslation("Enter name of new directory"));
        
    if (!newdir || !newdir[0])
        return;
        
    newdir = CreateText("%s/%s", dialog->m_pDirChooser->directory(), newdir);
    
    try
    {
        if (MKDirNeedsRoot(newdir))
        {
            LIBSU::CLibSU suhandler;
            const char *passwd = dialog->AskPassword(suhandler);
            MKDirRecRoot(newdir, suhandler, passwd);
        }
        else
            MKDirRec(newdir);
            
        dialog->m_pDirChooser->directory(newdir);
    }
    catch(Exceptions::CExIO &e)
    {
        fl_alert(e.what());
    }
}
