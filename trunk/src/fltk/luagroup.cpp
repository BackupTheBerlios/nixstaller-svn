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
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>

// -------------------------------------
// Lua Widget Group Class
// -------------------------------------

CLuaGroup::CLuaGroup()
{
    m_pMainPack = new Fl_Pack(0, 0, 0, 0);
    m_pMainPack->resizable(NULL);
    m_pMainPack->type(Fl_Pack::HORIZONTAL);
    m_pMainPack->spacing(WidgetSpacing());
    m_pMainPack->end();
}

CBaseLuaCFGMenu *CLuaGroup::CreateCFGMenu(const char *desc)
{
    CLuaCFGMenu *ret = new CLuaCFGMenu(desc);
    AddWidget(ret);
    return ret;
}

CBaseLuaCheckbox *CLuaGroup::CreateCheckbox(const char *desc, const std::vector<std::string> &l)
{
    CLuaCheckbox *ret = new CLuaCheckbox(desc, l);
    AddWidget(ret);
    return ret;
}

CBaseLuaDirSelector *CLuaGroup::CreateDirSelector(const char *desc, const char *val)
{
    CLuaDirSelector *ret = new CLuaDirSelector(desc, val);
    AddWidget(ret);
    return ret;
}

CBaseLuaImage *CLuaGroup::CreateImage(const char *file)
{
    CLuaImage *ret = new CLuaImage(file);
    AddWidget(ret);
    return ret;
}

CBaseLuaInputField *CLuaGroup::CreateInputField(const char *label, const char *desc, const char *val,
                                                int max, const char *type)
{
    CLuaInputField *ret = new CLuaInputField(label, desc, val, max, type);
    AddWidget(ret);
    return ret;
}

CBaseLuaLabel *CLuaGroup::CreateLabel(const char *title)
{
    CLuaLabel *ret = new CLuaLabel(title);
    AddWidget(ret);
    return ret;
}

CBaseLuaMenu *CLuaGroup::CreateMenu(const char *desc, const std::vector<std::string> &l)
{
    CLuaMenu *ret = new CLuaMenu(desc, l);
    AddWidget(ret);
    return ret;
}

CBaseLuaProgressBar *CLuaGroup::CreateProgressBar(const char *desc)
{
    CLuaProgressBar *ret = new CLuaProgressBar(desc);
    AddWidget(ret);
    return ret;
}

CBaseLuaRadioButton *CLuaGroup::CreateRadioButton(const char *desc, const std::vector<std::string> &l)
{
    CLuaRadioButton *ret = new CLuaRadioButton(desc, l);
    AddWidget(ret);
    return ret;
}

CBaseLuaTextField *CLuaGroup::CreateTextField(const char *desc, bool wrap)
{
    CLuaTextField *ret = new CLuaTextField(desc, wrap);
    AddWidget(ret);
    return ret;
}

void CLuaGroup::AddWidget(CLuaWidget *w)
{
    w->GetGroup()->user_data(w);
    m_pMainPack->add(w->GetGroup());
}

CLuaWidget *CLuaGroup::GetWidget(Fl_Widget *w)
{
    return static_cast<CLuaWidget *>(w->user_data());
}

int CLuaGroup::ExpandedWidgets()
{
    const int size = m_pMainPack->children();
    int ret = 0;
    for (int i=0; i<size; i++)
    {
        if (!m_pMainPack->child(i)->visible())
            continue;
        
        CLuaWidget *w = GetWidget(m_pMainPack->child(i));
        if (w->Expand())
            ret++;
    }
    
    return ret;
}

int CLuaGroup::RequestedWidgetsW()
{
    const int size = m_pMainPack->children();
    int ret = 0;
    for (int i=0; i<size; i++)
    {
        if (!m_pMainPack->child(i)->visible())
            continue;

        CLuaWidget *w = GetWidget(m_pMainPack->child(i));
        ret += w->RequestWidth();
        
        if (i != (size-1))
            ret += WidgetSpacing();
    }
    
    return ret;
}

int CLuaGroup::TotalWidgetHeight(int maxw)
{
    const int size = m_pMainPack->children();
    int ret = 0;
    for (int i=0; i<size; i++)
    {
        if (!m_pMainPack->child(i)->visible())
            continue;

        CLuaWidget *w = GetWidget(m_pMainPack->child(i));
        ret = std::max(ret, w->RequestHeight(maxw));
    }
    
    return ret;
}

Fl_Group *CLuaGroup::GetGroup()
{
    return m_pMainPack;
}

void CLuaGroup::SetSize(int maxw, int maxh)
{
    maxw -= WidgetSpacing(); // HACK: FLTK adds spacing at the beginning (bug)
    
    const int expsize = ExpandedWidgets();
    const int diffw = maxw - RequestedWidgetsW();
    const int totalwidgeth = TotalWidgetHeight(maxw);
    const int extraw = (expsize) ? diffw / expsize : 0;
    const int size = m_pMainPack->children();
    
    if (totalwidgeth > m_pMainPack->h())
        m_pMainPack->size(m_pMainPack->w(), totalwidgeth);

    for (int i=0; i<size; i++)
    {
        if (!m_pMainPack->child(i)->visible())
            continue;

        CLuaWidget *widget = GetWidget(m_pMainPack->child(i));
        int w = widget->RequestWidth();
        
        if (widget->Expand())
            w += extraw;
        
        assert(w <= maxw);
        
        widget->SetSize(w, totalwidgeth);
//         widget->GetGroup()->position(widget->GetGroup()->x(),
//                                      widget->GetGroup()->y() + ((totalwidgeth - widget->GetGroup()->h()) / 2));
    }
}
