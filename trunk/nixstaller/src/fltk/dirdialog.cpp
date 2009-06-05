/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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

#include <FL/Fl_File_Chooser.H>

#include "main/main.h"
#include "main/frontend/suterm.h"
#include "main/frontend/utils.h"
#include "dirdialog.h"

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

const char *CFLTKDirDialog::AskPassword(CSuTerm *suterm)
{
    const char *ret = NULL;
    
    while (true)
    {
        ret = fl_password(GetTranslation("Your account doesn't have permissions to "
                "create the directory.\nTo create it with the root "
                "(administrator) account, please enter the administrative password below."));

        if (!ret || !ret[0])
            break;

        try
        {
            if (!suterm->TestPassword(ret))
                fl_alert(GetTranslation("Incorrect password given, please retype."));
            else
                break;
        }
        catch (Exceptions::CExIO &e)
        {
            fl_alert(e.what());
            break;
        }
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
            CSuTerm suterm;
            const char *passwd = dialog->AskPassword(&suterm);
            MKDirRecRoot(newdir, &suterm, passwd);
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
