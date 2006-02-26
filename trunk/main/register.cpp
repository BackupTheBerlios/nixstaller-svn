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
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "main.h"

CRegister Register;

const char *CRegister::GetFileName()
{
    if (!m_szFileName)
    {
        const char *home = getenv("HOME");
        if (!home)
            throwerror(false, "Couldn't find out your home directory!");
        
        if (mkdir(CreateText("%s/.nixstaller", home), (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
            throwerror(false, "Could not create nixstaller config directory!(%s)", strerror(errno));

        m_szFileName = CreateText("%s/.nixstaller/register", home);
    }
    
    return m_szFileName;
}

bool CRegister::IsInstalled(bool checkver)
{
    std::ifstream file(GetFileName());
    std::string line, str;
    
    while (file)
    {
        if (!std::getline(file, line))
            break;

        std::istringstream strstrm(line);

        if (!(strstrm >> str))
            break;

        if (str.compare("name"))
            continue;

        std::getline(strstrm, str);
        EatWhite(str);

        if (str == InstallInfo.program_name)
        {
            if (checkver)
            {
                if (!std::getline(file, line))
                    break;

                std::istringstream strstrm(line);

                if (!(strstrm >> str))
                    break;

                if (str.compare("version"))
                    break;

                std::getline(strstrm, str);
                if (EatWhite(str) == InstallInfo.version)
                    return true;
                
                break;
            }
            else
                return true;
        }
    }

    return false;
}

void CRegister::RemoveFromRegister(void)
{
    if (!IsInstalled(true))
        return;

    std::ifstream infile(GetFileName());
    std::string buffer, line;
    
    if (!infile)
        return;
    
    while(infile)
    {
        std::getline(infile, line);
        debugline("line : %s\n", line.c_str());

        if (line.compare(0, 5, "name ") == 0)
        {
            if (line.compare(5, std::string::npos, InstallInfo.program_name) == 0)
            {
                std::string tmp;
                std::getline(infile, tmp);
                if (!tmp.compare(0, 8, "version ") && !tmp.compare(8, std::string::npos, InstallInfo.version))
                    continue;

                line += "\n" + tmp;
            }
        }
        buffer += line + "\n";
    }

    infile.close();
    
    std::ofstream outfile(GetFileName());
    
    if (!outfile)
        return;

    outfile << buffer;
    debugline(buffer.c_str());
}

void CRegister::RegisterInstall(void)
{
    if (IsInstalled(true))
        return;
    
    std::ofstream file(GetFileName(), std::ofstream::app);
    
    if (!file)
        throwerror(false, "Error while opening register file");

    file << "name " + InstallInfo.program_name + "\n";
    file << "version " + InstallInfo.version + "\n";
}
