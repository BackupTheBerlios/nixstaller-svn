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

#include "main/frontend/utils.h"
#include "luacfgmenu.h"
#include "tui/button.h"
#include "tui/dialog.h"
#include "tui/menu.h"
#include "tui/textfield.h"
#include "tui/tui.h"

// -------------------------------------
// Lua Config Menu Class
// -------------------------------------

CLuaCFGMenu::CLuaCFGMenu(const char *desc) : CBaseLuaWidget(desc)
{
    NNCurses::CBox *box = new NNCurses::CBox(NNCurses::CBox::HORIZONTAL, false, 1);
    
    box->AddWidget(m_pMenu = new NNCurses::CMenu(20, 7));
    box->AddWidget(m_pDescWin = new NNCurses::CTextField(25, 7, true));
    m_pDescWin->SetMinWidth(15);
    
    AddWidget(box);
}

void CLuaCFGMenu::CoreAddVar(const char *name)
{
    m_pMenu->AddEntry(name, GetTranslation(name));
}

void CLuaCFGMenu::CoreDelVar(const char *name)
{
    m_pMenu->DelEntry(name);
    UpdateDesc();
}

void CLuaCFGMenu::CoreUpdateVar(const char *name)
{
}

void CLuaCFGMenu::CoreUpdateLanguage()
{
    for (TVarType::iterator it=GetVariables().begin(); it!=GetVariables().end(); it++)
        m_pMenu->SetName(it->first, GetTranslation(it->first));
    UpdateDesc();
}

CLuaCFGMenu::SEntry *CLuaCFGMenu::GetCurEntry()
{
    return GetVariables()[m_pMenu->Value()];
}

void CLuaCFGMenu::UpdateDesc()
{
    if (m_pMenu->Empty())
        return;
    
    SEntry *entry = GetCurEntry();
    
    if (entry)
        m_pDescWin->SetText(GetTranslation(entry->desc));
}

void CLuaCFGMenu::ShowChoiceMenu()
{
    SEntry *entry = GetCurEntry();
    std::string item = m_pMenu->Value();
    NNCurses::TColorPair fc(COLOR_GREEN, COLOR_BLUE), dfc(COLOR_WHITE, COLOR_BLUE);
    const char *msg = CreateText(GetTranslation("Please choose a new value for %s"), GetTranslation(item.c_str()));
    NNCurses::CDialog *dialog = NNCurses::CreateBaseDialog(fc, dfc, 20, 0, msg);
    
    NNCurses::CMenu *menu = new NNCurses::CMenu(15, 8);
    
    for (TOptionsType::const_iterator it=entry->options.begin(); it!=entry->options.end(); it++)
        menu->AddEntry(*it, GetTranslation(*it));
    
    if (!entry->val.empty())
        menu->Select(entry->val);
    
    dialog->AddWidget(menu);
    
    dialog->AddButton(new NNCurses::CButton(GetTranslation("OK")), true, false);
    
    NNCurses::CButton *cancelbutton = new NNCurses::CButton(GetTranslation("Cancel"));
    dialog->AddButton(cancelbutton, true, false);
    
    NNCurses::TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    if (dialog->ActivatedWidget() != cancelbutton)
        entry->val = menu->Value();
    
    delete dialog;
}

bool CLuaCFGMenu::CoreHandleEvent(NNCurses::CWidget *emitter, int event)
{
    if (CLuaWidget::CoreHandleEvent(emitter, event))
        return true;
    
    if (m_pMenu->Empty())
        return false;

    if (emitter == m_pMenu)
    {
        if (event == EVENT_DATACHANGED)
        {
            UpdateDesc();
            return true;
        }
        else if (event == EVENT_CALLBACK)
        {
            SEntry *entry = GetCurEntry();
            std::string item = m_pMenu->Value();
    
            if (entry)
            {
                switch (entry->type)
                {
                    case TYPE_STRING:
                    {
                        std::string ret = NNCurses::InputBox(CreateText(GetTranslation("Please enter a new value for %s"),
                                                             GetTranslation(item.c_str())), entry->val);
                        if (!ret.empty())
                            entry->val = ret;
                        break;
                    }
                    case TYPE_DIR:
                    {
                        std::string ret = NNCurses::DirectoryBox(CreateText(GetTranslation("Please choose a new value for %s"),
                                                             GetTranslation(item.c_str())), entry->val);
                        if (!ret.empty())
                            entry->val = ret;
                        break;
                    }
                    case TYPE_LIST:
                    case TYPE_BOOL:
                        ShowChoiceMenu();
                        break;
                }
                
                LuaDataChanged();
                return true;
            }
        }
    }
    
    return false;
}

void CLuaCFGMenu::CoreGetButtonDescs(NNCurses::TButtonDescList &list)
{
    list.push_back(NNCurses::TButtonDescPair("ENTER", "Change"));
    CLuaWidget::CoreGetButtonDescs(list);
}
