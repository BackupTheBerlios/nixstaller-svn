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

#ifdef WITH_APPMANAGER
#include "fltk.h"
#include <FL/x.H>

CAppManager::CAppManager()
{
    const int mainw = 560, mainh = 330;
    m_pMainWindow = new Fl_Window(mainw, mainh, "Nixstaller - App Manager");

    m_pAppList = new Fl_Hold_Browser(20, 40, 180, 240, "Installed applications");
    m_pAppList->align(FL_ALIGN_TOP);
    GetRegisterEntries(&m_AppVec);
    for (std::vector<app_entry_s *>::iterator it=m_AppVec.begin(); it!=m_AppVec.end(); it++)
        m_pAppList->add(MakeCString((*it)->name));
    m_pAppList->callback(AppListCB, this);
    m_pAppList->value(1);

    Fl_Tabs *pTabs = new Fl_Tabs((m_pAppList->x()+m_pAppList->w())+20, 20, 300, 260);
    
    Fl_Group *pInfoTab = new Fl_Group((m_pAppList->x()+m_pAppList->w())+20, 40, 300, 260, "Info");
    m_pInfoOutput = new Fl_Help_View((m_pAppList->x()+m_pAppList->w())+20, 40, 300, 240);
    m_pInfoOutput->align(FL_ALIGN_TOP);
    pInfoTab->end();
    
    Fl_Group *pFilesTab = new Fl_Group((m_pAppList->x()+m_pAppList->w())+20, 40, 300, 260, "Files");
    m_pFilesTextDisplay = new Fl_Text_Display((m_pAppList->x()+m_pAppList->w())+20, 40, 300, 240);
    m_pFilesTextBuffer = new Fl_Text_Buffer;
    m_pFilesTextDisplay->buffer(m_pFilesTextBuffer);
    
    pFilesTab->end();
    
    pTabs->end();
    m_pUninstallButton = new Fl_Button(20, (m_pAppList->y()+m_pAppList->h())+20, 120, 25, "Uninstall");
    m_pUninstallButton->callback(UninstallCB, this);
    m_pExitButton = new Fl_Button((mainw-140), (m_pAppList->y()+m_pAppList->h())+20, 120, 25, "Exit");
    m_pExitButton->callback(ExitCB);
    
    UpdateInfo(true);

    m_pUninstallWindow = new CUninstallWindow(this);
    
    m_pMainWindow->end();
}

void CAppManager::AddUninstOutput(const std::string &str)
{
    m_pUninstallWindow->AddOutput(str);
}

void CAppManager::SetProgress(int percent)
{
    m_pUninstallWindow->SetProgress(percent);
}
    
void CAppManager::UpdateInfo(bool init)
{
    if (m_AppVec.empty())
    {
        m_pInfoOutput->value("");
        m_pFilesTextBuffer->select(0, m_pFilesTextBuffer->length());
        m_pFilesTextBuffer->remove_selection();
        return;
    }
    
    const char *format =
            "<center><h3><b>%s</b></h3></center><br><br>"
            "<table><tbody>"
            "<tr><td><b>Version</b></td><td>%s</td></tr>"
            "<tr><td><b>Web site</b></td><td>%s</td></tr>"
            "<tr><td><b>Description</b></td><td><pre>%s</pre></td></tr>"
            "</tbody></table>";
    
    if (init)
        m_pCurrentAppEntry = m_AppVec.front();
    else
    {
        int index = m_pAppList->value() - 1;
        if ((index >= 0) && (index < m_AppVec.size()))
            m_pCurrentAppEntry = m_AppVec[index];
        else
            return;
    }
    
    m_pInfoOutput->value(CreateText(format, m_pCurrentAppEntry->name.c_str(), m_pCurrentAppEntry->version.c_str(),
                                    m_pCurrentAppEntry->url.c_str(), m_pCurrentAppEntry->description.c_str()));
    
    // Clear previous contents
    m_pFilesTextBuffer->select(0, m_pFilesTextBuffer->length());
    m_pFilesTextBuffer->remove_selection();
    
    for (std::map<std::string, std::string>::iterator it=m_pCurrentAppEntry->FileSums.begin();
         it!=m_pCurrentAppEntry->FileSums.end(); it++)
    {
        m_pFilesTextBuffer->append(it->first.c_str());
        m_pFilesTextBuffer->append("\n");
    }
}

void CAppManager::StartUninstall()
{
    if (m_AppVec.empty())
        return;

    if (!m_pUninstallWindow->Start(m_pCurrentAppEntry))
        return;
    
    m_pAppList->remove(m_pAppList->value());
    
    std::vector<app_entry_s *>::iterator it = std::find(m_AppVec.begin(), m_AppVec.end(), m_pCurrentAppEntry);
    if (it != m_AppVec.end())
        m_AppVec.erase(it);
    
    fl_message(GetTranslation("Uninstallation of %s complete!"), m_pCurrentAppEntry->name.c_str());

    delete m_pCurrentAppEntry;
    UpdateInfo(false);
}


CUninstallWindow::CUninstallWindow(CAppManager *owner) : m_pOwner(owner)
{
    const int w = 400, h = 300;
    m_pWindow = new Fl_Window(w, h, "Uninstallation progress");
    m_pWindow->set_modal();
    
    m_pProgress = new Fl_Progress(50, 20, (w-100), 30, "Progress");
    m_pProgress->minimum(0);
    m_pProgress->maximum(100);
    m_pProgress->value(0);
    
    m_pBuffer = new Fl_Text_Buffer;
    
    m_pDisplay = new Fl_Text_Display(50, 80, (w-100), (h-140), "Status");
    m_pDisplay->buffer(m_pBuffer);
    m_pDisplay->wrap_mode(true, 60);
    
    m_pOKButton = new Fl_Button((w-80)/2, (h-40), 80, 25, GetTranslation("OK"));
    m_pOKButton->callback(OKButtonCB, this);
    m_pOKButton->deactivate();
    
    m_pWindow->end();
}

bool CUninstallWindow::Start(app_entry_s *pApp)
{
    if (!fl_choice(CreateText(GetTranslation("This will remove %s from your computer\nContinue?"),
         pApp->name.c_str()), GetTranslation("No"), GetTranslation("Yes"), NULL))
        return false;
    
    bool checksums = false;
    if (!m_pOwner->CheckSums(pApp->name.c_str()))
    {
        int ret = fl_choice(GetTranslation("Some files have been modified after installation.\n"
                "This can happen if you installed another package which uses one or\n"
                "more files with the same name or you installed another version."),
                GetTranslation("Cancel"), GetTranslation("Continue anyway"),
                GetTranslation("Only remove unchanged"));
        if (ret == 0)
            return false;
        
        checksums = (ret == 2);
    }
    
    // Clear previous contents
    m_pBuffer->select(0, m_pBuffer->length());
    m_pBuffer->remove_selection();
    
    m_pProgress->value(0);
    
    m_pWindow->show();
    
    m_pOwner->Uninstall(pApp, checksums);
    
    m_pProgress->value(100);
    m_pOKButton->activate();
    return true;
}

void CUninstallWindow::AddOutput(const std::string &file)
{
    m_pBuffer->append(CreateText("Removing %s\n", file.c_str()));
    // Move cursor to last line and last word
    m_pDisplay->scroll(m_pBuffer->length(), m_pDisplay->word_end(m_pBuffer->length()));
    Fl::wait(0.0); // Update screen
}
#endif
