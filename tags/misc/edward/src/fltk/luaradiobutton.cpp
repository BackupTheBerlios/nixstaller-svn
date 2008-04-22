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
#include "luaradiobutton.h"
#include <FL/Fl_Group.H>
#include <FL/Fl_Round_Button.H>
#include <FL/fl_draw.H>

// -------------------------------------
// Lua Checkbox Class
// -------------------------------------

CLuaRadioButton::CLuaRadioButton(const char *desc, const TOptions &l) : CBaseLuaWidget(desc), CBaseLuaRadioButton(l)
{
    TSTLVecSize size = l.size(), n;
    
    for (n=0; n<size; n++)
    {
        Fl_Round_Button *button = new Fl_Round_Button(0, 0, 0, ButtonHeight(), MakeTranslation(l[n]));
        button->type(FL_RADIO_BUTTON);
        button->callback(ToggleCB, this);
        
        if (m_RadioButtons.empty())
            button->setonly();
        
        m_RadioButtons.push_back(button);
        GetGroup()->add(button);
    }
}

const char *CLuaRadioButton::EnabledButton()
{
    TOptions &opts = GetOptions();
    TSTLVecSize size = opts.size();
    for (TSTLVecSize n=0; n<size; n++)
    {
        if (m_RadioButtons[n]->value())
            return opts.at(n).c_str();
    }
    
    assert(false);
    return 0; // Shouldn't happen...
}

void CLuaRadioButton::Enable(TSTLVecSize n)
{
    m_RadioButtons.at(n)->setonly();
}

void CLuaRadioButton::CoreUpdateLanguage()
{
    TOptions &opts = GetOptions();
    TSTLVecSize size = opts.size(), n;
    
    for (n=0; n<size; n++)
        m_RadioButtons[n]->label(MakeTranslation(opts[n]));
}

int CLuaRadioButton::CoreRequestWidth()
{
    TSTLVecSize size = m_RadioButtons.size();
    int ret = 0;
    
    if (!size)
        return 0;
    
    fl_font(m_RadioButtons[0]->labelfont(), m_RadioButtons[0]->labelsize());
    
    for (TSTLVecSize n=0; n<size; n++)
    {
        ret = std::max(ret, static_cast<int>(fl_width(m_RadioButtons[n]->label())));
    }
    
    return ret;
}

int CLuaRadioButton::CoreRequestHeight(int maxw)
{
    const int size = SafeConvert<int>(m_RadioButtons.size());
    if (size)
        return (size*ButtonHeight());
    
    return 0;
}

void CLuaRadioButton::UpdateSize()
{
    TSTLVecSize size = m_RadioButtons.size();
    for (TSTLVecSize n=0; n<size; n++)
        m_RadioButtons[n]->size(GetGroup()->w(), m_RadioButtons[n]->h());
}

void CLuaRadioButton::ToggleCB(Fl_Widget *w, void *p)
{
    CLuaRadioButton *box = static_cast<CLuaRadioButton *>(p);
    box->LuaDataChanged();
}
