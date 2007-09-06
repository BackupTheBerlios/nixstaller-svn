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
#include "luainput.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Pack.H>
#include <FL/fl_draw.H>

// -------------------------------------
// Lua FLTK InputField Class
// -------------------------------------

CLuaInputField::CLuaInputField(const char *label, const char *desc, const char *val, int max,
                               const char *type) : CBaseLuaWidget(desc), CBaseLuaInputField(label, type), m_pLabel(NULL)
{
    const int inputh = 25;
    m_pPack = new Fl_Pack(0, 0, 0, inputh);
    m_pPack->resizable(NULL);
    m_pPack->type(Fl_Pack::HORIZONTAL);
    m_pPack->spacing(PackSpacing());
    m_pPack->begin();
    
    if (label && *label)
    {
        m_pLabel = new Fl_Box(0, 0, 0, inputh, GetTranslation(label));
        m_pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    }
    
    if (GetType() == "number")
        m_pInputField = new Fl_Int_Input(0, 0, 0, inputh);
    else if (GetType() == "float")
        m_pInputField = new Fl_Float_Input(0, 0, 0, inputh);
    else
        m_pInputField = new Fl_Input(0, 0, 0, inputh);
    
    m_pInputField->maximum_size(max);
    m_pInputField->callback(InputChangedCB, this);
    m_pInputField->when(FL_WHEN_CHANGED);
    
    if (val && *val)
        m_pInputField->value(val);
    
    m_pPack->end();
    GetGroup()->add(m_pPack);
}

const char *CLuaInputField::CoreGetValue()
{
    return m_pInputField->value();
}

void CLuaInputField::CoreUpdateLanguage()
{
    if (m_pLabel)
    {
        m_pLabel->label(MakeTranslation(GetLabel()));
        UpdateSize();
    }
}

int CLuaInputField::CoreRequestWidth()
{
    int minw = 50;
    
    if (m_pLabel)
    {
        fl_font(m_pLabel->labelfont(), m_pLabel->labelsize());
        minw += static_cast<int>(fl_width(m_pLabel->label()));
    }
    
    return minw;
}

int CLuaInputField::CoreRequestHeight(int maxw)
{
    return m_pInputField->h();
}

void CLuaInputField::UpdateSize()
{
    m_pPack->size(GetGroup()->w(), m_pPack->h());
    
    if (GetLabel().empty() || !m_pLabel)
    {
        m_pInputField->size(GetGroup()->w(), m_pInputField->h());
        return;
    }
    
    TSTLStrSize max = SafeConvert<TSTLStrSize>(GetLabelWidth()), length = GetLabel().length();
    
    if (length > max)
    {
        std::string label = GetTranslation(GetLabel()).substr(0, max);
        m_pLabel->label(MakeCString(label));
    }
    else if (length < max)
    {
        std::string label = GetTranslation(GetLabel());
        label.append(max - length, ' ');
        m_pLabel->label(MakeCString(label));
    }
    
    fl_font(m_pLabel->labelfont(), m_pLabel->labelsize());
    int w = GetGroup()->w(), h = 0;
    fl_measure(m_pLabel->label(), w, h);
    m_pLabel->size(w, m_pLabel->h());
    
    m_pInputField->size(GetGroup()->w() - PackSpacing() - w, m_pInputField->h());
    
    debugline("label w: %d - y: %d with '%s'\n", m_pLabel->w(), m_pLabel->h(), m_pLabel->label());
}

void CLuaInputField::InputChangedCB(Fl_Widget *w, void *p)
{
    CLuaInputField *input = static_cast<CLuaInputField *>(p);
    input->LuaDataChanged();
}
