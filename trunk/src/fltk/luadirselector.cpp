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

#include "fltk.h"
#include "dirdialog.h"
#include "luadirselector.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>

// -------------------------------------
// Lua directory selector Class
// -------------------------------------

CLuaDirSelector::CLuaDirSelector(const char *desc, const char *val) : CBaseLuaWidget(desc)
{
    const int dirinputh = 35, buttonh = 25;
    
    // Group for placing widgets next to eachother
    Fl_Group *dirgroup = new Fl_Group(0, 0, 0, dirinputh);
    dirgroup->resizable(NULL);
    GetGroup()->add(dirgroup);
    
    m_pDirInput = new Fl_File_Input(0, 0, 0, dirinputh);
    m_pDirInput->callback(InputChangedCB, this);
    
    if (val && *val)
        m_pDirInput->value(val);
    
    m_pBrowseButton = new Fl_Button(0, (dirinputh-buttonh)/2, 0, buttonh, GetTranslation("Browse"));
    m_pBrowseButton->callback(BrowseCB, this);
    
    dirgroup->end();
}

const char *CLuaDirSelector::GetDir(void)
{
    return m_pDirInput->value();
}

void CLuaDirSelector::SetDir(const char *dir)
{
    m_pDirInput->value(dir);
}

void CLuaDirSelector::CoreUpdateLanguage()
{
    m_pBrowseButton->label(GetTranslation("Browse"));
    UpdateLayout();
}

int CLuaDirSelector::CoreRequestHeight(int maxw)
{
    return m_pDirInput->h();
}

void CLuaDirSelector::UpdateLayout()
{
    SetButtonWidth(m_pBrowseButton);
    
    int inputw = GetGroup()->w() - Spacing() - m_pBrowseButton->w();
    m_pDirInput->size(inputw, m_pDirInput->h());
    m_pBrowseButton->position(m_pDirInput->x() + inputw + Spacing(), m_pBrowseButton->y());
}

void CLuaDirSelector::InputChangedCB(Fl_Widget *w, void *p)
{
    CLuaDirSelector *dirselector = static_cast<CLuaDirSelector *>(p);
    dirselector->LuaDataChanged();
}

void CLuaDirSelector::BrowseCB(Fl_Widget *w, void *p)
{
    CLuaDirSelector *dirsel = static_cast<CLuaDirSelector *>(p);
    CFLTKDirDialog dialog("~", "*", (Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE),
             GetTranslation("Select a directory"));
    
    dialog.Value(dirsel->m_pDirInput->value());
    dialog.Run();
    
    const char *dir = dialog.Value();
    if (dir && *dir)
    {
        dirsel->m_pDirInput->value(dir);
        dirsel->LuaDataChanged();
    }
}
