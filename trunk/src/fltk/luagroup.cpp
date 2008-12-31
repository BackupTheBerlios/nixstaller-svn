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
#include "fltk.h"
#include "installscreen.h"
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

CLuaGroup::CLuaGroup(CInstallScreen *screen) : m_iMaxWidth(0), m_pInstallScreen(screen)
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

CBaseLuaCheckbox *CLuaGroup::CreateCheckbox(const char *desc, const std::vector<std::string> &l,
                                            const std::vector<TSTLVecSize> &e)
{
    CLuaCheckbox *ret = new CLuaCheckbox(desc, l, e);
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

CBaseLuaMenu *CLuaGroup::CreateMenu(const char *desc, const std::vector<std::string> &l,
                                    TSTLVecSize e)
{
    CLuaMenu *ret = new CLuaMenu(desc, l, e);
    AddWidget(ret);
    return ret;
}

CBaseLuaProgressBar *CLuaGroup::CreateProgressBar(const char *desc)
{
    CLuaProgressBar *ret = new CLuaProgressBar(desc);
    AddWidget(ret);
    return ret;
}

CBaseLuaRadioButton *CLuaGroup::CreateRadioButton(const char *desc, const std::vector<std::string> &l,
                                                  TSTLVecSize e)
{
    CLuaRadioButton *ret = new CLuaRadioButton(desc, l, e);
    AddWidget(ret);
    return ret;
}

CBaseLuaTextField *CLuaGroup::CreateTextField(const char *desc, bool wrap, const char *size)
{
    CLuaTextField *ret = new CLuaTextField(desc, wrap, size);
    AddWidget(ret);
    return ret;
}

void CLuaGroup::AddWidget(CLuaWidget *w)
{
    w->SetParent(this);
    
    // The widget is put in another group, so that it can center vertically
    Fl_Group *group = new Fl_Group(0, 0, 0, 0);
    group->resizable(NULL);
    group->end();
    group->add(w->GetGroup());
    group->user_data(w);
    
    m_pMainPack->add(group);
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
        CLuaWidget *w = GetWidget(m_pMainPack->child(i));
        
        if (!w->GetGroup()->visible())
            continue;
        
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
        CLuaWidget *w = GetWidget(m_pMainPack->child(i));
        
        if (!w->GetGroup()->visible())
            continue;

        ret += w->RequestWidth();
        
        if (i != (size-1))
            ret += WidgetSpacing();
    }
    
    return ret;
}

Fl_Group *CLuaGroup::GetGroup()
{
    return m_pMainPack;
}

void CLuaGroup::UpdateLayout()
{
    const int expsize = ExpandedWidgets();
    const int diffw = m_iMaxWidth - RequestedWidgetsW();
    const int extraw = (expsize) ? diffw / expsize : 0;
    const int size = m_pMainPack->children();
    int totalwidgeth = 0;
    
    // First give each widget a size...
    for (int i=0; i<size; i++)
    {
        CLuaWidget *widget = GetWidget(m_pMainPack->child(i));
        
        if (!widget->GetGroup()->visible())
            continue;

        int w = widget->RequestWidth();
        
        if (widget->Expand())
            w += extraw;
        
        assert(w <= m_iMaxWidth);
        
        int h = widget->RequestHeight(w);
        totalwidgeth = std::max(totalwidgeth, h);
        widget->SetSize(w, h);
    }
    
    // Now position every widget
    for (int i=0; i<size; i++)
    {
        CLuaWidget *widget = GetWidget(m_pMainPack->child(i));
        
        if (!widget->GetGroup()->visible())
            continue;

        Fl_Widget *wgroup = m_pMainPack->child(i);
        wgroup->size(widget->GetGroup()->w(), totalwidgeth);
        
        widget->GetGroup()->position(wgroup->x(), wgroup->y() + ((totalwidgeth - widget->GetGroup()->h()) / 2));
        
        widget->GetGroup()->redraw();
    }

    if (totalwidgeth > m_pMainPack->h())
        m_pMainPack->size(m_pMainPack->w(), totalwidgeth);
    
    m_pInstallScreen->UpdateSubScreens();
//     m_pMainPack->parent()->redraw(); // Important (still?)
}

void CLuaGroup::SetSize(int maxw, int maxh)
{
    maxw -= WidgetSpacing(); // HACK: FLTK adds spacing at the beginning (bug)
    m_iMaxWidth = maxw;
    UpdateLayout();
}

