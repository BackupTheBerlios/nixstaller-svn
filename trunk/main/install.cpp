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

#include <fstream>
#include "main.h"

void CBaseInstall::InitArchive(const char *archname)
{
    if (!FileExists(archname))
        return;
        
    char *fname = CreateText("%s.sizes", archname);
    std::ifstream file(fname);
    std::string arfilename;
    unsigned int size;

    // Read first column to size and the other column(s) to arfilename
    while(file && (file >> size) && std::getline(file, arfilename))
    {
        EatWhite(arfilename);
        m_ArchList[const_cast<char*>(archname)].filesizes[arfilename] = size;
        m_iTotalArchSize += size;
    }
}

bool CBaseInstall::ExtractFiles()
{
    // Init
    m_iTotalArchSize = 0;
    m_fExtrPercent = 0.0f;
    m_ArchList.clear();
    m_szCurArchFName = NULL;
    
    // Init all archives (if file doesn't exist nothing will be done)
    InitArchive(CreateText("%s/instarchive_all", InstallInfo.own_dir.c_str()));
    InitArchive(CreateText("%s/instarchive_all_%s", InstallInfo.own_dir.c_str(), InstallInfo.cpuarch.c_str()));
    InitArchive(CreateText("%s/instarchive_%s", InstallInfo.own_dir.c_str(), InstallInfo.os.c_str()));
    InitArchive(CreateText("%s/instarchive_%s_%s", InstallInfo.own_dir.c_str(), InstallInfo.os.c_str(),
                InstallInfo.cpuarch.c_str()));
    
    if (m_ArchList.empty())
        return true; // No files to extract

    bool needroot = false;
    
    // Set up su
    /*m_SUHandler.SetUser("root");
    m_SUHandler.SetOutputFunc(SUOutFunc, this);
    m_SUHandler.SetTerminalOutput(false);*/

    while (m_CurArchIter != m_ArchList.end())
    {
        m_szCurArchFName = m_CurArchIter->first;
        
        // Set extract command
        std::string command = "cat " + std::string(m_szCurArchFName);

        if (InstallInfo.archive_type == ARCH_GZIP)
            command += " | gzip -cd | tar xvf -";
        else if (InstallInfo.archive_type == ARCH_BZIP2)
            command += " | bzip2 -d | tar xvf -";

        if (needroot)
        {
            /*m_SUHandler.SetCommand(command);
            if (!m_SUHandler.ExecuteCommand(passwd))
            return false;*/
        }
        else
        {
            command += " 2>&1"; // tar may output files to stderr
            
            FILE *pipe = popen(command.c_str(), "r");
            if (!pipe)
            {
                // UNDONE
                return false;
            }
            
            char line[512];
            while (fgets(line, sizeof(line), pipe))
            {
                std::string stat = line;
                
                if (stat.compare(0, 2, "x ") == 0)
                    stat.erase(0, 2);
                               
                EatWhite(stat);

                AddStatusText("Extracting file: " + stat);
                
                stat += '\n'; // File names are stored with a newline
                
                m_fExtrPercent += ((float)m_ArchList[m_szCurArchFName].filesizes[stat]/(float)m_iTotalArchSize)*100.0f;
                SetProgress((int)m_fExtrPercent);
            }
        }
        m_CurArchIter++;
    }
    return true;
}
