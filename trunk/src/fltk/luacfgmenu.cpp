/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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

#include "fltk.h"
#include "dirdialog.h"
#include "luacfgmenu.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>

// -------------------------------------
// Lua Config Menu Class
// -------------------------------------

CLuaCFGMenu::CLuaCFGMenu(const char *desc) : CBaseLuaWidget(desc)
{
    m_ColumnWidths[0] = m_ColumnWidths[1] = 0;
    
    const int inputh = 25, dirinputh = 35, inputspacing = 10;
    GetGroup()->begin();
    
    m_pVarListView = new Fl_Hold_Browser(0, 0, 0, GroupHeight() - (inputspacing+dirinputh));
    m_pVarListView->callback(SelectionCB, this);
    
    m_pInputField = new Fl_Input(0, 0, 0, inputh);
    m_pInputField->callback(InputChangedCB, this);
    m_pInputField->when(FL_WHEN_CHANGED);
    m_pInputField->hide();
    
    m_pChoiceMenu = new Fl_Choice(0, 0, 0, inputh);
    m_pChoiceMenu->callback(ChoiceChangedCB, this);
    m_pChoiceMenu->hide();
    
    // Group for placing widgets next to eachother
    Fl_Group *dirgroup = new Fl_Group(0, 0, 0, dirinputh);
    dirgroup->resizable(NULL);
    
    m_pDirInput = new Fl_File_Input(0, 0, 0, dirinputh);
    m_pDirInput->callback(InputChangedCB, this);
    m_pDirInput->when(FL_WHEN_CHANGED);
    m_pDirInput->hide();
    
    m_pBrowseButton = new Fl_Button(0, (dirinputh-inputh) / 2, 0, inputh, GetTranslation("Browse"));
    SetButtonWidth(m_pBrowseButton);
    m_pBrowseButton->callback(BrowseCB, this);
    m_pBrowseButton->hide();
    
    dirgroup->end();
    
    GetGroup()->end();
}

void CLuaCFGMenu::CoreAddVar(const char *name)
{
    SetVarColumnW(name);
    m_pVarListView->add(ColumnText(name), (void *)name);
    
    if (m_pVarListView->size() == 1) // Init menu: highlight first item
    {
        m_pVarListView->value(1);
        UpdateSelection();
    }
}

void CLuaCFGMenu::CoreDelVar(const char *name)
{
    const int size = m_pVarListView->size();
    int index = -1;
    
    for (int i=1; i<=size; i++)
    {
        if (m_pVarListView->data(i) && !strcmp(static_cast<const char *>(m_pVarListView->data(i)), name))
        {
            index = i;
            break;
        }
    }
    
    if (index != -1)
    {
        m_pVarListView->remove(index);
        TVarType &vars = GetVariables();
        for (TVarType::iterator it=vars.begin(); it!=vars.end(); it++)
            SetVarColumnW(it->first.c_str());
        UpdateSelection();
    }
}

void CLuaCFGMenu::CoreUpdateVar(const char *name)
{
    UpdateSelection();
}

void CLuaCFGMenu::CoreUpdateLanguage()
{
    m_pBrowseButton->label(GetTranslation("Browse"));
    UpdateDirChooser(GetGroup()->w());
    
    // Update var menu
    const int size = m_pVarListView->size();
    for (int n=1; n<=size; n++)
    {
        const char *name = static_cast<const char *>(m_pVarListView->data(n));
        SetVarColumnW(name);
        m_pVarListView->text(n, ColumnText(name));
    }
}

void CLuaCFGMenu::UpdateSize()
{
    int w = GetGroup()->w();
    m_pVarListView->size(w, m_pVarListView->h());
    m_pInputField->size(w, m_pInputField->h());
    m_pChoiceMenu->size(w, m_pChoiceMenu->h());
    UpdateDirChooser(w);
}

void CLuaCFGMenu::SetVarColumnW(const char *var)
{
    const int varspacing = 10;
    const char *trname = GetTranslation(var);
    
    fl_font(m_pVarListView->labelfont(), m_pVarListView->labelsize());
    int w = fl_width(trname) + varspacing;
    
    if (w > m_ColumnWidths[0])
    {
        m_ColumnWidths[0] = w;
        m_pVarListView->column_widths(m_ColumnWidths);
        m_pVarListView->redraw(); // Make's sure that widget uses new column width
    }
}

const char *CLuaCFGMenu::ColumnText(const char *var)
{
    return CreateText("%s\t%s", GetTranslation(var), GetTranslation(GetVariables()[var]->desc.c_str()));
}

const char *CLuaCFGMenu::CurSelection()
{
    const char *item = static_cast<const char *>(m_pVarListView->data(m_pVarListView->value()));
    if (!item || !*item || !GetVariables()[item])
        return NULL;
    
    return item;
}

void CLuaCFGMenu::UpdateDirChooser(int w)
{
    SetButtonWidth(m_pBrowseButton);
    
    int inputw = w - DirChooserSpacing() - m_pBrowseButton->w();
    m_pDirInput->size(inputw, m_pDirInput->h());
    m_pBrowseButton->position(m_pDirInput->x() + inputw + DirChooserSpacing(), m_pBrowseButton->y());
}

void CLuaCFGMenu::UpdateSelection()
{
    const char *var = CurSelection();
    if (!var)
        return;
    
    m_pInputField->hide();
    m_pChoiceMenu->hide();
    m_pDirInput->hide();
    m_pBrowseButton->hide();

    const SEntry *entry = GetVariables()[var];
    
    switch(entry->type)
    {
        case TYPE_STRING:
            m_pInputField->value(entry->val.c_str());
            m_pInputField->show();
            break;
        case TYPE_DIR:
            m_pDirInput->value(entry->val.c_str());
            m_pDirInput->show();
            m_pBrowseButton->show();
            break;
        case TYPE_LIST:
        case TYPE_BOOL:
        {
            m_pChoiceMenu->clear();
            
            TOptionsType::size_type cur = 0, n = 0;
            for (TOptionsType::const_iterator it=entry->options.begin(); it!=entry->options.end(); it++)
            {
                m_pChoiceMenu->add(GetTranslation(it->c_str()));
                if (!cur && (*it == entry->val))
                    cur = n;
                n++;
            }
            
            if (m_pChoiceMenu->size())
                m_pChoiceMenu->value(SafeConvert<int>(cur));
            
            m_pChoiceMenu->show();
            break;
        }
    }
}

void CLuaCFGMenu::InputChangedCB(Fl_Widget *w, void *p)
{
    CLuaCFGMenu *menu = static_cast<CLuaCFGMenu *>(p);
    const char *selection = menu->CurSelection();

    if (selection)
    {
        menu->GetVariables()[selection]->val = (static_cast<Fl_Input *>(w))->value();
        menu->LuaDataChanged();
    }
}

void CLuaCFGMenu::ChoiceChangedCB(Fl_Widget *w, void *p)
{
    CLuaCFGMenu *menu = static_cast<CLuaCFGMenu *>(p);
    const char *selection = menu->CurSelection();

    if (selection)
    {
        SEntry *entry = menu->GetVariables()[selection];
        int n = menu->m_pChoiceMenu->value();
        entry->val = entry->options[n];
        menu->LuaDataChanged();
    }
}

void CLuaCFGMenu::BrowseCB(Fl_Widget *w, void *p)
{
    CLuaCFGMenu *menu = static_cast<CLuaCFGMenu *>(p);
    const char *selection = menu->CurSelection();
    SEntry *entry = menu->GetVariables()[selection];
    CFLTKDirDialog dialog("~", "*", (Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE),
                          CreateText(GetTranslation("Please choose a new value for %s"),
                                     GetTranslation(selection)));
    
    dialog.Value(menu->m_pDirInput->value());
    dialog.Run();
    
    const char *dir = dialog.Value();
    if (dir && *dir)
    {
        entry->val = dir;
        menu->m_pDirInput->value(dir);
        menu->LuaDataChanged();
    }
}

