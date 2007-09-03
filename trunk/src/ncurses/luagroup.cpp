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
#include "luawidget.h"
#include "tui/tui.h"

// -------------------------------------
// Lua Widget Group Class
// -------------------------------------

CBaseLuaInputField *CLuaGroup::CreateInputField(const char *label, const char *desc, const char *val,
                                                int max, const char *type)
{
    CLuaInputField *ret = new CLuaInputField(label, desc, val, max, type);
    AddLuaWidget(ret);
    return ret;
}

CBaseLuaCheckbox *CLuaGroup::CreateCheckbox(const char *desc, const std::vector<std::string> &l)
{
    CLuaCheckbox *ret = new CLuaCheckbox(desc, l);
    AddLuaWidget(ret);
    return ret;
}

CBaseLuaRadioButton *CLuaGroup::CreateRadioButton(const char *desc, const std::vector<std::string> &l)
{
    CLuaRadioButton *ret = new CLuaRadioButton(desc, l);
    AddLuaWidget(ret);
    return ret;
}

CBaseLuaDirSelector *CLuaGroup::CreateDirSelector(const char *desc, const char *val)
{
    CLuaDirSelector *ret = new CLuaDirSelector(desc, val);
    AddLuaWidget(ret);
    return ret;
}

CBaseLuaCFGMenu *CLuaGroup::CreateCFGMenu(const char *desc)
{
    CLuaCFGMenu *ret = new CLuaCFGMenu(desc);
    AddLuaWidget(ret);
    return ret;
}

CBaseLuaMenu *CLuaGroup::CreateMenu(const char *desc, const std::vector<std::string> &l)
{
    CLuaMenu *ret = new CLuaMenu(desc, l);
    AddLuaWidget(ret);
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
    AddLuaWidget(ret);
    return ret;
}

CBaseLuaTextField *CLuaGroup::CreateTextField(const char *desc, bool wrap)
{
    CLuaTextField *ret = new CLuaTextField(desc, wrap);
    AddLuaWidget(ret);
    return ret;
}

CBaseLuaLabel *CLuaGroup::CreateLabel(const char *title)
{
    CLuaLabel *ret = new CLuaLabel(title);
    AddLuaWidget(ret);
    return ret;
}

void CLuaGroup::AddLuaWidget(CLuaWidget *w)
{
    if (m_bInitEnable)
    {
        m_bInitEnable = false;
        Enable(true);
    }
    
    // Update each widget's max size, this is necessary before adding new widget because they
    // may request too much width
    const TWidgetList &list = GetWidgetList();
    const int size = SafeConvert<int>(list.size()) + 1; // +1 because new widget is not added yet
    const int maxwidgetsw = NNCurses::GetMaxWidth() - 6;
    const int maxwidth = (maxwidgetsw - (LuaWidgetSpacing() * (size-1)) ) / size;
    for (TWidgetList::const_iterator it=list.begin(); it!=list.end(); it++)
    {
        CLuaWidget *luaw = dynamic_cast<CLuaWidget *>(*it);
        if (luaw)
            luaw->SetMaxWidth(maxwidth);
    }

    CBox *box = new CBox(VERTICAL, false);
    box->StartPack(w, true, false, 0, 0);
    w->SetMaxWidth(maxwidth);

    StartPack(box, true, true, 0, 0);
}
