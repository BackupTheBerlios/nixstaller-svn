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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "tui.h"
#include "filedialog.h"
#include "label.h"
#include "menu.h"
#include "inputfield.h"
#include "button.h"

namespace {

class CNCursUserMKDir: public CFrontendMKDirHelper
{
    virtual void WarnBox(const char *msg) { NNCurses::WarningBox(msg); }
    virtual char *AskPassword(const char *msg)
    {
        return StrDup(NNCurses::InputBox(GetTranslation("Your account doesn't have permissions to "
                "create the directory. To create it with the root (administrator) account, please"
                "enter it's password below."), "", 0, '*').c_str());
    }
};

}


namespace NNCurses {

// -------------------------------------
// File Dialog Class (Currently only handles directories)
// -------------------------------------

CFileDialog::CFileDialog(const std::string &msg, const std::string &start) : m_bCancelled(true)
{
    m_Directory = (start.empty()) ? "/" : GetFirstValidDir(start);
    
    StartPack((m_pLabel = new CLabel(msg)), true, true, 1, 1);
    StartPack(m_pFileMenu = new CMenu(25, 8), true, true, 0, 0);
    StartPack((m_pInputField = new CInputField(m_Directory, CInputField::STRING, 1024)), false, false, 1, 0);
    AddButton(m_pOKButton = new CButton(GetTranslation("Open directory")), true, false);
    AddButton(m_pCancelButton = new CButton(GetTranslation("Cancel")), true, false);
    
    OpenDir(m_Directory);
}

bool CFileDialog::ValidDir(CDirIter &dir)
{
    bool isrootdir = (m_Directory == "/");
    struct stat filestat;
    
    return ((lstat(dir->d_name, &filestat) == 0) && S_ISDIR(filestat.st_mode) &&
            (!isrootdir || strcmp(dir->d_name, "..")));
}

void CFileDialog::OpenDir(const std::string &newdir)
{
    try
    {
        CHDir(newdir);
        m_Directory = GetCWD();
        
        CDirIter dir(m_Directory);
    
        m_pFileMenu->ClearEntries();
        
        while (dir)
        {
            // Valid directory?
            if (ValidDir(dir))
                m_pFileMenu->AddEntry(dir->d_name);
            
            dir++;
        }
        
        m_pInputField->SetText(m_Directory);
    }
    catch(Exceptions::CExReadDir &e)
    {
        // Couldn't open directory(probably no read access)
        WarningBox(e.what());
        
        // If no directory is open yet, just open /
        if (m_pFileMenu->Empty())
        {
            if (newdir == "/")
                throw; // Seems we can't even open / ...
            
            OpenDir("/");
        }
    }
}

bool CFileDialog::CoreHandleKey(chtype key)
{
    if (CDialog::CoreHandleKey(key))
        return true;
    
    if (key == KEY_F(2))
    {
        std::string newdir = InputBox(GetTranslation("Enter name of new directory"), "", 1024);
        
        if (newdir.empty())
            return true;
        
        CNCursUserMKDir dirmaker;
        if (dirmaker(newdir))
            OpenDir(newdir);
            
        return true;
    }
    
    return false;
}

bool CFileDialog::CoreHandleEvent(CWidget *emitter, int type)
{
    if (type == EVENT_CALLBACK)
    {
        if ((emitter == m_pFileMenu) && !m_pFileMenu->Empty())
        {
            OpenDir(m_Directory + "/" + m_pFileMenu->Value());
            return true;
        }
        else if (emitter == m_pInputField)
        {
            OpenDir(m_pInputField->Value());
            return true;
        }
        else if (emitter == m_pOKButton)
        {
            m_bCancelled = false;
            // Don't return true, as the parent should recieve the event aswell
        }
    }
    
    return CDialog::CoreHandleEvent(emitter, type);
}

void CFileDialog::CoreGetButtonDescs(TButtonDescList &list)
{
    list.push_back(TButtonDescPair("F2", "Create new directory"));
    CDialog::CoreGetButtonDescs(list);
}

std::string CFileDialog::Value() const
{
    if (m_bCancelled)
        return "";
    
    return m_Directory;
}


}
