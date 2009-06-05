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

#ifndef FILEDIALOG
#define FILEDIALOG

#include <string>
#include "dialog.h"

namespace LIBSU {
class CLibSU;
}

class CDirIter;
class CSuTerm;

namespace NNCurses {

class CLabel;
class CMenu;
class CInputField;
class CButton;

class CFileDialog: public CDialog
{
    std::string m_Directory;
    CLabel *m_pLabel;
    CMenu *m_pFileMenu;
    CInputField *m_pInputField;
    CButton *m_pOKButton, *m_pCancelButton;
    bool m_bCancelled;
    
    bool ValidDir(CDirIter &dir);
    void OpenDir(std::string newdir);
    std::string AskPassword(CSuTerm *suterm);
    
protected:
    virtual bool CoreHandleKey(wchar_t key);
    virtual bool CoreHandleEvent(CWidget *emitter, int event);
    virtual void CoreGetButtonDescs(TButtonDescList &list);
    
public:
    CFileDialog(const std::string &msg, const std::string &start);
    
    std::string Value(void) const;
};

}


#endif
