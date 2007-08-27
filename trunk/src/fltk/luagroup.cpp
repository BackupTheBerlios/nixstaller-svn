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
#include "fltk.h"
#include "luagroup.h"
// #include "luacfgmenu.h"
// #include "luacheckbox.h"
// #include "luadirselector.h"
// #include "luaimage.h"
// #include "luainput.h"
#include "lualabel.h"
// #include "luamenu.h"
// #include "luaprogressbar.h"
// #include "luaradiobutton.h"
// #include "luatextfield.h"
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>

// -------------------------------------
// Lua Widget Group Class
// -------------------------------------

CLuaGroup::CLuaGroup(int w)
{
    m_pMainPack = new Fl_Pack(0, 0, w, 0);
    m_pMainPack->type(Fl_Pack::HORIZONTAL);
    m_pMainPack->end();
}

CBaseLuaLabel *CLuaGroup::CreateLabel(const char *title)
{
    CLuaLabel *ret = new CLuaLabel(title);
    AddWidget(ret);
    return ret;
}

void CLuaGroup::AddWidget(CLuaWidget *w)
{
    if (w->GetGroup()->h() > m_pMainPack->h())
        m_pMainPack->size(m_pMainPack->w(), w->GetGroup()->h());

    m_pMainPack->add(w->GetGroup());
}

Fl_Group *CLuaGroup::GetGroup()
{
    return m_pMainPack;
}
