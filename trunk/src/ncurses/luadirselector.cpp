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

#include "luadirselector.h"
#include "tui/button.h"
#include "tui/dialog.h"
#include "tui/inputfield.h"

// -------------------------------------
// Lua Config Menu Class
// -------------------------------------

CLuaDirSelector::CLuaDirSelector(const char *desc, const char *val) : CLuaWidget(desc)
{
    NNCurses::CBox *box = new NNCurses::CBox(NNCurses::CBox::HORIZONTAL, false, 3);
    
    box->AddWidget(m_pDirInput = new NNCurses::CInputField(val, NNCurses::CInputField::STRING));
    box->StartPack(m_pBrowseButton = new NNCurses::CButton(GetTranslation("Browse")), false, false, 0, 0);
    
    StartPack(box, false, false, 0, 0);
}

const char *CLuaDirSelector::GetDir(void)
{
    return m_pDirInput->Value().c_str();
}

void CLuaDirSelector::SetDir(const char *dir)
{
    m_pDirInput->SetText(dir);
}

bool CLuaDirSelector::CoreHandleEvent(NNCurses::CWidget *emitter, int event)
{
    if ((event == EVENT_CALLBACK) && (emitter == m_pBrowseButton))
    {
        std::string ret = NNCurses::DirectoryBox(GetTranslation("Select directory"), m_pDirInput->Value());
        if (!ret.empty())
            m_pDirInput->SetText(ret);
        return true;
    }
    
    return false;
}

void CLuaDirSelector::CoreUpdateLanguage()
{
    m_pBrowseButton->SetText(GetTranslation("Browse"));
}
