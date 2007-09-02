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

#include "fltk.h"
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

CLuaCFGMenu::CLuaCFGMenu(const char *desc) : CBaseLuaWidget(desc), m_pDirChooser(NULL)
{
    m_ColumnWidths[0] = m_ColumnWidths[1] = 0;
    
    const int grouph = 175, inputh = 25, dirinputh = 35, inputspacing = 10;
    GetGroup()->size(GetGroup()->w(), grouph);
    GetGroup()->begin();
    
    m_pVarListView = new Fl_Hold_Browser(0, 0, 0, grouph - (inputspacing+dirinputh));
    m_pVarListView->callback(SelectionCB, this);
    
    int y = grouph - inputh;
    m_pInputField = new Fl_Input(0, y, 0, inputh);
    m_pInputField->callback(InputChangedCB, this);
    m_pInputField->hide();
    
    m_pChoiceMenu = new Fl_Choice(0, y, 0, inputh);
    m_pChoiceMenu->callback(ChoiceChangedCB, this);
    m_pChoiceMenu->hide();
    
    // Group for placing widgets next to eachother
    Fl_Group *dirgroup = new Fl_Group(0, y, 0, dirinputh);
    dirgroup->resizable(NULL);
    
    m_pDirInput = new Fl_File_Input(0, y, 0, dirinputh);
    m_pDirInput->callback(InputChangedCB, this);
    m_pDirInput->hide();
    
    y += ((dirinputh-inputh)/2); // Center
    m_pBrowseButton = new Fl_Button(0, y, 0, inputh, GetTranslation("Browse"));
    SetButtonWidth(m_pBrowseButton);
    m_pBrowseButton->callback(BrowseCB, this);
    m_pBrowseButton->hide();
    
    dirgroup->end();
    
    GetGroup()->end();
    
    CreateDirSelector();
    UpdateDirChooser();

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

void CLuaCFGMenu::CoreUpdateLanguage()
{
    UpdateDirChooser();
    CreateDirSelector(); // Recreate, so that it will use the new translations
    
    // Update var menu
    const int size = m_pVarListView->size();
    for (int n=1; n<=size; n++)
    {
        const char *name = static_cast<const char *>(m_pVarListView->data(n));
        SetVarColumnW(name);
        m_pVarListView->text(n, ColumnText(name));
    }
}

void CLuaCFGMenu::CoreSetSize(int maxw, int maxh)
{
    GetGroup()->size(maxw, GetGroup()->h());
    m_pVarListView->size(maxw, m_pVarListView->h());
    m_pInputField->size(maxw, m_pInputField->h());
    m_pChoiceMenu->size(maxw, m_pChoiceMenu->h());
    UpdateDirChooser();
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

void CLuaCFGMenu::CreateDirSelector()
{
    if (m_pDirChooser)
        delete m_pDirChooser;
    
    m_pDirChooser = new Fl_File_Chooser("~", "*",
                                        (Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE),
                                         GetTranslation("Select a directory"));
    m_pDirChooser->preview(false);
    m_pDirChooser->previewButton->hide();
    m_pDirChooser->newButton->tooltip(Fl_File_Chooser::new_directory_tooltip);
    m_pDirChooser->newButton->callback(MKDirCB, this); // Hook in our own nice directory creation function :)
}

void CLuaCFGMenu::UpdateDirChooser()
{
    m_pBrowseButton->label(GetTranslation("Browse"));
    SetButtonWidth(m_pBrowseButton);
    
    int inputw = GetGroup()->w() - DirChooserSpacing() - m_pBrowseButton->w();
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

const char *CLuaCFGMenu::AskPassword(LIBSU::CLibSU &suhandler)
{
    const char *ret = NULL;
    
    while (true)
    {
        ret = fl_password(GetTranslation("Your account doesn't have permissions to "
                "create the directory. To create it with the root "
                "(administrator) account, please enter it's password below."));

        if (!ret || !ret[0])
            break;

        if (!suhandler.TestSU(ret))
        {
            if (suhandler.GetError() == LIBSU::CLibSU::SU_ERROR_INCORRECTPASS)
                fl_alert(GetTranslation("Incorrect password given for root user\nPlease retype"));
            else
            {
                fl_alert(GetTranslation("Could not use su to gain root access. "
                        "Make sure you can use su (adding the current user to the wheel group may help)"));
                break;
            }
        }
        else
            break;
    }
    
    return ret;
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
    
    menu->m_pDirChooser->directory(GetFirstValidDir(entry->val).c_str());
    menu->m_pDirChooser->show();
    while(menu->m_pDirChooser->visible())
        Fl::wait();
    
    const char *newdir = menu->m_pDirChooser->value();
    if (newdir && *newdir)
    {
        entry->val = newdir;
        menu->LuaDataChanged();
        menu->m_pDirInput->value(newdir);
    }
}

void CLuaCFGMenu::MKDirCB(Fl_Widget *w, void *p)
{
    CLuaCFGMenu *menu = static_cast<CLuaCFGMenu *>(p);
    const char *newdir = fl_input(GetTranslation("Enter name of new directory"));
        
    if (!newdir || !newdir[0])
        return;
        
    newdir = CreateText("%s/%s", menu->m_pDirChooser->directory(), newdir);
    
    try
    {
        if (MKDirNeedsRoot(newdir))
        {
            LIBSU::CLibSU suhandler;
            const char *passwd = menu->AskPassword(suhandler);
            MKDirRecRoot(newdir, suhandler, passwd);
        }
        else
            MKDirRec(newdir);
            
        menu->m_pDirChooser->directory(newdir);
    }
    catch(Exceptions::CExIO &e)
    {
        fl_alert(e.what());
    }
}
