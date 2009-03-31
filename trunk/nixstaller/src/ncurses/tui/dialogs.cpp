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
#include "tui.h"
#include "dialog.h"
#include "label.h"
#include "button.h"
#include "inputfield.h"
#include "filedialog.h"

namespace {

int MaxW(void)
{
    return NNCurses::GetMaxWidth() - 6;
}

}

namespace NNCurses {

// -------------------------------------
// Dialog Util Functions
// -------------------------------------

CDialog *CreateBaseDialog(TColorPair fc, TColorPair dfc, int minw, int minh, const std::string &text)
{
    CDialog *dialog = new CDialog;
    dialog->SetFColors(fc);
    dialog->SetDFColors(dfc);

    if (minw)
        dialog->SetMinWidth(minw);

    if (minh)
        dialog->SetMinHeight(minh);

    if (!text.empty())
    {
        CLabel *label = new CLabel(text);
        label->SetMaxReqWidth(MaxW());
        dialog->StartPack(label, true, true, 1, 1);
    }
    
    return dialog;
}

void MessageBox(const std::string &msg)
{
    CDialog *dialog = CreateBaseDialog(TColorPair(COLOR_GREEN, COLOR_BLUE),
                                       TColorPair(COLOR_WHITE, COLOR_BLUE), 25, 0, msg);
    
    dialog->AddButton(new CButton(GetTranslation("OK")));
    
    TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    delete dialog;
}

void WarningBox(const std::string &msg)
{
    CDialog *dialog = CreateBaseDialog(TColorPair(COLOR_YELLOW, COLOR_RED),
                                       TColorPair(COLOR_WHITE, COLOR_RED), 30, 0, msg);
    
    dialog->AddButton(new CButton(GetTranslation("OK")));
    
    TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    delete dialog;
}

bool YesNoBox(const std::string &msg)
{
    CDialog *dialog = CreateBaseDialog(TColorPair(COLOR_GREEN, COLOR_BLUE), TColorPair(COLOR_WHITE, COLOR_BLUE),
                                       30, 0, msg);
    
    CButton *nobutton = new CButton(GetTranslation("No")), *yesbutton = new CButton(GetTranslation("Yes"));
    dialog->AddButton(yesbutton);
    dialog->AddButton(nobutton);
    
    TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    bool ret = (dialog->ActivatedWidget() == yesbutton);
    
    delete dialog;
    
    return ret;
}

std::string InputBox(const std::string &msg, const std::string &init, int max, chtype out)
{
    CDialog *dialog = CreateBaseDialog(TColorPair(COLOR_GREEN, COLOR_BLUE),
                                       TColorPair(COLOR_WHITE, COLOR_BLUE), 50);
    
    CLabel *label = new CLabel(msg, false);
    label->SetMaxReqWidth(MaxW());
    dialog->AddWidget(label);
    
    CInputField *input = new CInputField(init, CInputField::STRING, max, out);
    dialog->StartPack(input, true, true, 1, 0);
    
    CButton *okbutton = new CButton(GetTranslation("OK")), *cancelbutton = new CButton(GetTranslation("Cancel"));
    dialog->AddButton(okbutton, false, false);
    dialog->AddButton(cancelbutton, false, false);

    TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    std::string ret;
    if (dialog->ActivatedWidget() != cancelbutton)
        ret = input->Value();
    
    delete dialog;
    
    return ret;
}

std::string DirectoryBox(const std::string &msg, const std::string &start)
{
    CFileDialog *dialog = new CFileDialog(msg, start);
    dialog->SetFColors(COLOR_GREEN, COLOR_BLUE);
    dialog->SetDFColors(COLOR_WHITE, COLOR_BLUE);
    dialog->SetMinWidth(50);
    
    TUI.AddGroup(dialog, true);
    
    while (dialog->Run())
        ;
    
    std::string ret = dialog->Value();
    
    delete dialog;
    
    return ret;
}


}
