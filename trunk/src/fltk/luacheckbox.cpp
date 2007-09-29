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

#include "main/main.h"
#include "luacheckbox.h"
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

// -------------------------------------
// Lua Checkbox Class
// -------------------------------------

CLuaCheckbox::CLuaCheckbox(const char *desc, const TOptions &l) : CBaseLuaWidget(desc), CBaseLuaCheckbox(l)
{
    TSTLVecSize size = l.size(), n;
    
    for (n=0; n<size; n++)
    {
        Fl_Check_Button *button = new Fl_Check_Button(0, 0, 0, ButtonHeight(), MakeTranslation(l[n]));
        button->type(FL_TOGGLE_BUTTON);
        button->callback(ToggleCB, this);
        m_Checkboxes.push_back(button);
        GetGroup()->add(button);
    }
}

bool CLuaCheckbox::Enabled(TSTLVecSize n)
{
    return m_Checkboxes.at(n)->value();
}

void CLuaCheckbox::Enable(TSTLVecSize n, bool b)
{
    m_Checkboxes.at(n)->value(b);
}

void CLuaCheckbox::CoreUpdateLanguage()
{
    TOptions &opts = GetOptions();
    TSTLVecSize size = opts.size(), n;
    
    for (n=0; n<size; n++)
        m_Checkboxes[n]->label(MakeTranslation(opts[n]));
}

int CLuaCheckbox::CoreRequestWidth()
{
    TSTLVecSize size = m_Checkboxes.size();
    int ret = 0;
    
    if (!size)
        return 0;
    
    fl_font(m_Checkboxes[0]->labelfont(), m_Checkboxes[0]->labelsize());
    
    for (TSTLVecSize n=0; n<size; n++)
    {
        ret = std::max(ret, static_cast<int>(fl_width(m_Checkboxes[n]->label())));
    }
    
    return ret;
}

int CLuaCheckbox::CoreRequestHeight(int maxw)
{
    const int size = SafeConvert<int>(m_Checkboxes.size());
    if (size)
        return (size*ButtonHeight());

    return 0;
}

void CLuaCheckbox::UpdateSize()
{
    TSTLVecSize size = m_Checkboxes.size();
    for (TSTLVecSize n=0; n<size; n++)
        m_Checkboxes[n]->size(GetGroup()->w(), m_Checkboxes[n]->h());
}

void CLuaCheckbox::ToggleCB(Fl_Widget *w, void *p)
{
    CLuaCheckbox *box = static_cast<CLuaCheckbox *>(p);
    box->LuaDataChanged();
}
