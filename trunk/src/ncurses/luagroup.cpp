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
#include "luagroup.h"
#include "luacfgmenu.h"
#include "luacheckbox.h"
#include "luadirselector.h"
#include "luaimage.h"
#include "luainput.h"
#include "lualabel.h"
#include "luamenu.h"
#include "luaprogressbar.h"
#include "luaradiobutton.h"
#include "luatextfield.h"

// -------------------------------------
// Lua Widget Group Class
// -------------------------------------

CBaseLuaInputField *CLuaGroup::CreateInputField(const char *label, const char *desc, const char *val,
                                                int max, const char *type)
{
    CLuaInputField *ret = new CLuaInputField(label, desc, val, max, type);
    CBox::AddWidget(ret);
    return ret;
}

CBaseLuaCheckbox *CLuaGroup::CreateCheckbox(const char *desc, const std::vector<std::string> &l)
{
    CLuaCheckbox *ret = new CLuaCheckbox(desc, l);
    CBox::AddWidget(ret);
    return ret;
}

CBaseLuaRadioButton *CLuaGroup::CreateRadioButton(const char *desc, const std::vector<std::string> &l)
{
    CLuaRadioButton *ret = new CLuaRadioButton(desc, l);
    CBox::AddWidget(ret);
    return ret;
}

CBaseLuaDirSelector *CLuaGroup::CreateDirSelector(const char *desc, const char *val)
{
    CLuaDirSelector *ret = new CLuaDirSelector(desc, val);
    CBox::AddWidget(ret);
    return ret;
}

CBaseLuaCFGMenu *CLuaGroup::CreateCFGMenu(const char *desc)
{
    CLuaCFGMenu *ret = new CLuaCFGMenu(desc);
    CBox::AddWidget(ret);
    return ret;
}

CBaseLuaMenu *CLuaGroup::CreateMenu(const char *desc, const std::vector<std::string> &l)
{
    CLuaMenu *ret = new CLuaMenu(desc, l);
    CBox::AddWidget(ret);
    return ret;
}

CBaseLuaImage *CLuaGroup::CreateImage(const char *file)
{
    // No widget addition: ncurses doesn't do gfx :)
    return new CLuaImage;
}

CBaseLuaProgressBar *CLuaGroup::CreateProgressBar(const char *desc)
{
    CLuaProgressBar *ret = new CLuaProgressBar(desc);
    CBox::AddWidget(ret);
    return ret;
}

CBaseLuaTextField *CLuaGroup::CreateTextField(const char *desc, bool wrap)
{
    CLuaTextField *ret = new CLuaTextField(desc, wrap);
    CBox::AddWidget(ret);
    return ret;
}

CBaseLuaLabel *CLuaGroup::CreateLabel(const char *title)
{
    CLuaLabel *ret = new CLuaLabel(title);
    CBox::AddWidget(ret);
    return ret;
}

void CLuaGroup::CoreAddWidget(NNCurses::CWidget *w)
{
    if (m_bInitEnable)
    {
        m_bInitEnable = false;
        Enable(true);
    }
    
    CBox::CoreAddWidget(w);
}
