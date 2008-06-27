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
#include "luaprogressdialog.h"
#include "tui/button.h"
#include "tui/label.h"
#include "tui/progressbar.h"

// -------------------------------------
// NCurses Lua Progress Dialog Class
// -------------------------------------

CLuaProgressDialog::CLuaProgressDialog(int r) : CBaseLuaProgressDialog(r)
{
    AddWidget(m_pTitle = new NNCurses::CLabel("", false));
    AddWidget(m_pProgBar = new NNCurses::CProgressBar(0.0f, 100.0f));
    AddWidget(m_pSecTitle = new NNCurses::CLabel("", false));
    AddWidget(m_pSecProgBar = new NNCurses::CProgressBar(0.0f, 100.0f));
    
    NNCurses::CBox *bbox = new NNCurses::CBox(NNCurses::CBox::HORIZONTAL, false);
    AddWidget(bbox);
    bbox->StartPack(m_pCancelButton = new NNCurses::CButton(GetTranslation("Cancel")), true, false, 0, 0);
    
    m_pCancelButton->Enable(false);
    m_pSecTitle->Enable(false);
    m_pSecProgBar->Enable(false);
}

void CLuaProgressDialog::CoreSetTitle(const char *title)
{
    m_pTitle->SetText(GetTranslation(title));
}

void CLuaProgressDialog::CoreSetProgress(int progress)
{
    m_pProgBar->SetCurrent(progress);
}

void CLuaProgressDialog::CoreEnableSecProgBar(bool enable)
{
    m_pSecTitle->Enable(enable);
    m_pSecProgBar->Enable(enable);
}

void CLuaProgressDialog::CoreSetSecTitle(const char *title)
{
    m_pSecTitle->SetText(GetTranslation(title));
}

void CLuaProgressDialog::CoreSetSecProgress(int progress)
{
    m_pSecProgBar->SetCurrent(progress);
}

void CLuaProgressDialog::CoreSetCancelButton(bool e)
{
    m_pCancelButton->Enable(e);
}

bool CLuaProgressDialog::CoreHandleEvent(NNCurses::CWidget *emitter, int type)
{
    if (NNCurses::CWindow::CoreHandleEvent(emitter, type))
        return true;
    
    if ((type == EVENT_CALLBACK) && (emitter == m_pCancelButton))
    {
        SetCancelled();
        return true;
    }
    
    return false;
}
