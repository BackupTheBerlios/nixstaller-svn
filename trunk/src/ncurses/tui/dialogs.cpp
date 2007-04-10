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

#include "tui.h"
#include "dialog.h"
#include "label.h"
#include "button.h"
#include "inputfield.h"

namespace NNCurses {

// -------------------------------------
// Dialog Util Functions
// -------------------------------------

void MessageBox(const std::string &msg)
{
    CDialog *dialog = new CDialog;
    dialog->SetFColors(COLOR_YELLOW, COLOR_BLUE);
    dialog->SetDFColors(COLOR_WHITE, COLOR_BLUE);
    
    dialog->AddWidget(new CLabel(msg));
    
    dialog->AddButton(new CButton("OK"));
    
    TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    TUI.RemoveGroup(dialog);
}

void WarningBox(const std::string &msg)
{
    CDialog *dialog = new CDialog;
    dialog->SetFColors(COLOR_YELLOW, COLOR_RED);
    dialog->SetDFColors(COLOR_WHITE, COLOR_RED);
    
    dialog->AddWidget(new CLabel(msg));
    
    dialog->AddButton(new CButton("OK"));
    
    TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    TUI.RemoveGroup(dialog);
}

bool YesNoBox(const std::string &msg)
{
    CDialog *dialog = new CDialog;
    dialog->SetFColors(COLOR_YELLOW, COLOR_BLUE);
    dialog->SetDFColors(COLOR_WHITE, COLOR_BLUE);
    
    dialog->AddWidget(new CLabel(msg));
        
    CButton *nobutton = new CButton("No"), *yesbutton = new CButton("Yes");
    dialog->AddButton(nobutton);
    dialog->AddButton(yesbutton);
    
    TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    bool ret = (dialog->ActivatedWidget() == yesbutton);
    
    TUI.RemoveGroup(dialog);
    
    return ret;
}

std::string InputBox(const std::string &msg, const std::string &init, int max, chtype out)
{
    CDialog *dialog = new CDialog;
    dialog->SetFColors(COLOR_YELLOW, COLOR_BLUE);
    dialog->SetDFColors(COLOR_WHITE, COLOR_BLUE);
    dialog->SetMinWidth(50);
    
    CLabel *label = new CLabel(msg, false);
    dialog->StartPack(label, false, false, 0, 0);
    
    CInputField *input = new CInputField(init, CInputField::STRING, max, out);
    dialog->StartPack(input, true, true, 1, 0);
    
    CButton *okbutton = new CButton("OK"), *cancelbutton = new CButton("Cancel");
    dialog->AddButton(cancelbutton, false, false);
    dialog->AddButton(okbutton, false, false);

    
    TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    std::string ret;
    if (dialog->ActivatedWidget() != cancelbutton)
        ret = input->GetText();
    
    TUI.RemoveGroup(dialog);
    
    return ret;
}

}
