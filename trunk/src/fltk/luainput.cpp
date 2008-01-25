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

#include <locale.h>
#include "main/main.h"
#include "luainput.h"
#include <FL/Fl.H>
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
        m_pLabel->labelfont(FL_COURIER); // Use a fixed font, so we can easily calc the width from GetLabelWidth()
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

void CLuaInputField::CoreSetValue(const char *v)
{
    m_Text = v;
    m_pInputField->value(v);
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
    
    std::string label = GetTranslation(GetLabel());
    TSTLStrSize max = SafeConvert<TSTLStrSize>(GetLabelWidth()), length = label.length();
    
    if (length > max)
        m_pLabel->label(MakeCString(label.substr(0, max)));
    
    fl_font(m_pLabel->labelfont(), m_pLabel->labelsize());
    int w = static_cast<int>(fl_width(' ')) * GetLabelWidth();
    m_pLabel->size(w, m_pLabel->h());
    
    m_pInputField->size(GetGroup()->w() - PackSpacing() - w, m_pInputField->h());
}

void CLuaInputField::InputChangedCB(Fl_Widget *w, void *p)
{
    CLuaInputField *input = static_cast<CLuaInputField *>(p);
    
    if ((input->GetType() == "number") || (input->GetType() == "float"))
    {
        // This code prevents multiple decimal points and plus/min signs, which the FLTK code unfortunaly doesn't
        std::string newtext = input->m_pInputField->value(), &oldtext = input->m_Text;
        
        TSTLStrSize size = newtext.length(), oldsize = oldtext.size();
        
        if (!size || (size < oldsize))
            input->m_Text = newtext;
        else
        {
            bool valid = true, updateddec = false;

            if (input->m_Text.empty())
                newtext = newtext;
            else
            {
                lconv *lc = localeconv();
        
                if (strchr(lc->decimal_point, ','))
                {
                    TSTLStrSize pos = newtext.find('.');
                    if (pos != std::string::npos)
                    {
                        newtext[pos] = ',';
                        updateddec = true;
                    }
                }
        
                TSTLStrSize start = newtext.find_first_of(lc->decimal_point);
                if ((start != std::string::npos) && (newtext.find_last_of(lc->decimal_point) != start))
                    valid = false;
                else
                {
                    const std::string plusmin = std::string(lc->positive_sign) +
                                                std::string(lc->negative_sign) + '+' + '-';
                    start = newtext.find_first_of(plusmin);
                    if ((start != std::string::npos) && (newtext.find_last_of(plusmin) != start))
                        valid = false;
                }
            }
            
            int cursorpos = input->m_pInputField->position();
            int oldcursorpos = cursorpos - SafeConvert<int>(size - oldsize);

            if (oldcursorpos < 0)
                oldcursorpos = 0;
            
            int newcursorpos;
            
            if (!valid)
            {
                input->m_pInputField->value(oldtext.c_str());
                newcursorpos = oldcursorpos;
            }
            else
            {
                input->m_Text = newtext;
                if (updateddec)
                    input->m_pInputField->value(newtext.c_str());
                newcursorpos = cursorpos;
            }
            
            // value() will reset the cursor position
            input->m_pInputField->position(newcursorpos);
        }
    }
    
    input->LuaDataChanged();
}
