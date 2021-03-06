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
#include "luamenu.h"
#include <FL/Fl_Hold_Browser.H>

// -------------------------------------
// Lua Menu Class
// -------------------------------------

CLuaMenu::CLuaMenu(const char *desc, const TOptions &l) : CBaseLuaWidget(desc), CBaseLuaMenu(l), m_iCurValue(-1)
{
    m_pMenu = new Fl_Hold_Browser(0, 0, 0, GroupHeight());
    m_pMenu->callback(SelectionCB, this);
    
    for (TOptions::const_iterator it=l.begin(); it!=l.end(); it++)
        m_pMenu->add(MakeTranslation(*it), MakeCString(*it));
    
    if (m_pMenu->size() > 0)
    {
        m_pMenu->value(1);
        m_iCurValue = 1;
    }
    
    GetGroup()->add(m_pMenu);
}

std::string CLuaMenu::Selection()
{
    const char *item = static_cast<const char *>(m_pMenu->data(m_pMenu->value()));
    
    assert(item && *item);
    
    if (!item || !*item)
        return "";
    
    return item;
}

void CLuaMenu::Select(TSTLVecSize n)
{
    m_pMenu->value(SafeConvert<int>(n)+1);
}

void CLuaMenu::CoreUpdateLanguage()
{
    const int size = m_pMenu->size();
    for (int i=1; i<=size; i++)
    {
        const char *name = static_cast<const char *>(m_pMenu->data(i));
        m_pMenu->text(i, MakeTranslation(name));
    }
}

void CLuaMenu::UpdateSize()
{
    m_pMenu->size(GetGroup()->w(), m_pMenu->h());
}

void CLuaMenu::SelectionCB(Fl_Widget *w, void *p)
{
    CLuaMenu *menu = static_cast<CLuaMenu *>(p);
    
    // HACK: Don't let the user "select nothing"
    if (menu->m_pMenu->value() == 0)
    {
        if (menu->m_iCurValue != -1)
            menu->m_pMenu->value(menu->m_iCurValue);
    }
    else
    {
        menu->m_iCurValue = menu->m_pMenu->value();
        menu->LuaDataChanged();
    }
}
