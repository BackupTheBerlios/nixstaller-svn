/*
    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.

    Nixstaller is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Nixstaller is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nixstaller; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
    conditions of the GNU General Public License cover the whole combination.

    In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
    software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
    DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
    such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
    you include the source code of that other code when and as the GNU GPL requires distribution of source code.

    Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
    versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
    version without this exception; this exception also makes it possible to release a modified version which carries forward
    this exception.
*/

#include "fltk.h"
#include <FL/x.H>

CAppManager::CAppManager(char **argv)
{
    const int mainw = 560, mainh = 330;
    m_pMainWindow = new Fl_Window(mainw, mainh, "Nixstaller - App Manager");

    m_pAppList = new Fl_Hold_Browser(20, 40, 180, 240, "Installed applications");
    m_pAppList->align(FL_ALIGN_TOP);
    Register.GetRegisterEntries(&m_AppVec);
    for (std::vector<app_entry_s *>::iterator it=m_AppVec.begin(); it!=m_AppVec.end(); it++)
        m_pAppList->add(MakeCString((*it)->name));
    m_pAppList->callback(AppListCB, this);
    m_pAppList->value(1);

    m_pInfoOutput = new Fl_Help_View((m_pAppList->x()+m_pAppList->w())+20, 40, 300, 240, "Info");
    m_pInfoOutput->align(FL_ALIGN_TOP);
    //m_pInfoOutput->textsize(13);

    m_pDeinstallButton = new Fl_Button(20, (m_pAppList->y()+m_pAppList->h())+20, 120, 25, "Deinstall");
    
    m_pExitButton = new Fl_Button((mainw-140), (m_pAppList->y()+m_pAppList->h())+20, 120, 25, "Exit");
    m_pExitButton->callback(ExitCB);
    
    UpdateInfo(true);

    m_pMainWindow->end();
    Run(argv);
}

void CAppManager::UpdateInfo(bool init)
{
    printf("MD5: %s\n", GetMD5("/usr/home/rick/operacodes.txt").c_str());
    const char *format =
            "<center><h3><b>%s</b></h3></center><br><br>"
            "<table><tbody>"
            "<tr><td><b>Version</b></td><td>%s</td></tr>"
            "<tr><td><b>Web site</b></td><td>%s</td></tr>"
            "<tr><td><b>Description</b></td><td>%s</td></tr>"
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
}
