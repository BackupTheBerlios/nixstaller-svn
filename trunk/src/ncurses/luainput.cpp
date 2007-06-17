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

#include "luainput.h"
#include "tui/box.h"
#include "tui/inputfield.h"
#include "tui/label.h"

// -------------------------------------
// Lua NCurses InputField Class
// -------------------------------------

CLuaInputField::CLuaInputField(const char *label, const char *desc, const char *val, int max,
                               const char *type) : CBaseLuaInputField(type), CLuaWidget(desc), m_pLabel(NULL)
{
    NNCurses::CBox *box = new NNCurses::CBox(NNCurses::CBox::HORIZONTAL, false, 1);
    
    if (label && *label)
    {
        m_Label = label;
        m_pLabel = new NNCurses::CLabel(GetTranslation(m_Label), false);
        m_pLabel->SetMaxReqHeight(1);
        box->StartPack(m_pLabel, false, false, 0, 0);
    }
    
    NNCurses::CInputField::EInputType t;
    
    if (GetType() == "string")
        t = NNCurses::CInputField::STRING;
    else if (GetType() == "number")
        t = NNCurses::CInputField::INT;
    else
        t = NNCurses::CInputField::FLOAT;
    
    box->AddWidget(m_pInputField = new NNCurses::CInputField((val) ? val : "", t, max));
    
    StartPack(box, false, false, 0, 0);
    UpdateLabelWidth();
}

void CLuaInputField::UpdateLabelWidth()
{
    if (m_pLabel)
    {
        int w = GetLabelWidth();
        m_pLabel->SetMinWidth(w);
        m_pLabel->SetMaxReqWidth(w);
        RequestUpdate();
    }
}

const char *CLuaInputField::CoreGetValue(void)
{
    return m_pInputField->Value().c_str();
}

void CLuaInputField::CoreUpdateLanguage()
{
    if (m_pLabel)
        m_pLabel->SetText(GetTranslation(m_Label));
}
