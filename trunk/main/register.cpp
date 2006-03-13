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
#include <dirent.h>

#include "main.h"

CRegister Register;

const char *CRegister::GetAppRegDir()
{
    if (!m_szConfDir)
    {
        const char *home = getenv("HOME");
        if (!home)
            throwerror(false, "Couldn't find out your home directory!");
        
        m_szConfDir = CreateText("%s/.nixstaller", home);
        
        if (mkdir(m_szConfDir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
            throwerror(false, "Could not create nixstaller config directory!(%s)", strerror(errno));
    }
    
    return m_szConfDir;
}

const char *CRegister::GetConfFile(const char *progname)
{
    const char *dir = CreateText("%s/%s", GetAppRegDir(), progname);
    if (mkdir(dir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
            throwerror(false, "Could not create nixstaller app-config directory!(%s)", strerror(errno));
    return CreateText("%s/config", dir);
}

const char *CRegister::GetSumListFile(const char *progname)
{
    const char *dir = CreateText("%s/%s", GetAppRegDir(), progname);
    if (mkdir(dir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
        throwerror(false, "Could not create nixstaller app-config directory!(%s)", strerror(errno));
    return CreateText("%s/list", dir);
}

app_entry_s *CRegister::GetAppEntry(const char *progname)
{
    const char *filename = GetConfFile(progname);
    if (!FileExists(filename))
        return NULL;

    app_entry_s *pAppEntry = new app_entry_s;
    pAppEntry->name = progname;
    
    std::ifstream file(filename);
    std::string line, str;
    
    while(file)
    {
        std::getline(file, line);
        std::istringstream strstrm(line);

        if (!(strstrm >> str))
            break;

        if (str == "version")
            std::getline(strstrm, pAppEntry->version);
        else if (str == "description") // UNDONE: need multiline support
            std::getline(strstrm, pAppEntry->description);
        else if (str == "url")
            std::getline(strstrm, pAppEntry->url);
    }

    file.close();

    filename = GetSumListFile(progname);
    if (FileExists(filename))
    {
        std::string sum;

        std::ifstream sumfile(filename);
        while(sumfile)
        {
            if (!(sumfile >> sum) || !std::getline(sumfile, line))
                break;
            pAppEntry->FileSums[EatWhite(line)] = sum;
        }
    }
    
    return pAppEntry;
}

void CRegister::WriteSums(const char *filename, std::ofstream &outfile, const std::string *var)
{
    std::ifstream infile(CreateText("%s/%s", InstallInfo.own_dir.c_str(), filename));
    std::string line;
    while (infile && std::getline(infile, line))
    {
        if (var)
            line.insert(0, *var + "/");
        else
            line.insert(0, InstallInfo.dest_dir + "/");
        
        outfile << GetMD5(line) << " " << line << "\n";
    }
}
    
bool CRegister::IsInstalled(bool checkver)
{
    if (!FileExists(GetConfFile(InstallInfo.program_name.c_str())))
        return false;
    
    app_entry_s *pApp = GetAppEntry(InstallInfo.program_name.c_str());
    
    if (!pApp)
        return false;
    
    return (!checkver || (pApp->version == InstallInfo.version));
}

void CRegister::RemoveFromRegister(app_entry_s *pApp)
{
    if (pApp->name.empty())
        return; // Otherwise we delete the main config directory ;)
        
    system(CreateText("rm -rf %s/%s", GetAppRegDir(), pApp->name.c_str())); // Lazy way to remove whole dir   
}

void CRegister::RegisterInstall(void)
{
    if (IsInstalled(true))
        return;
    
    std::ofstream file(GetConfFile(InstallInfo.program_name.c_str()));
    
    if (!file)
        throwerror(false, "Error while opening register file");

    file << "version " + InstallInfo.version + "\n";
    // UNDONE: Write description and url too
}

CRegister::EUninstRet CRegister::Uninstall(app_entry_s *pApp, bool checksum, TUpFunc UpFunc, TPasFunc PasFunc,
                                           void *pData)
{
    float percent = 0.0f;
    char *passwd = NULL;
    std::map<std::string, std::string>::iterator it;
    bool needroot = false;
    
    // Check if we got permission to remove all files
    for (it=pApp->FileSums.begin(); it!=pApp->FileSums.end(); it++)
    {
        if (FileExists(it->first) && (!WriteAccess(it->first) || !ReadAccess(it->first)))
        {
            needroot = true;
            break;
        }
    }
    
    if (needroot)
    {
        m_SUHandler.SetUser("root");
        m_SUHandler.SetTerminalOutput(false);
        
        if (m_SUHandler.NeedPassword())
        {
            passwd = PasFunc(pData);
            if (!passwd)
                return UNINST_NULLPASS;
            
            if (!m_SUHandler.TestSU(passwd))
            {
                // Some error appeared
                if (m_SUHandler.GetError() == LIBSU::CLibSU::SU_ERROR_INCORRECTPASS)
                    return UNINST_WRONGPASS;
                else
                    return UNINST_SUERR;
            }
        }
    }
    
    // Now get rid of the app...
    for (it=pApp->FileSums.begin(); it!=pApp->FileSums.end(); it++)
    {
        if (!FileExists(it->first))
        {
            debugline("Couldn't find file %s\n", it->first.c_str());
            continue;
        }
        
        if (checksum && (GetMD5(it->first) != it->second))
        {
            debugline("MD5 Mismatch: %s for %s\n", it->first.c_str(), pApp->name.c_str());
            continue;
        }
        
        if (needroot)
        {
            m_SUHandler.SetCommand(CreateText("rm %s", it->first.c_str()));
            if (!m_SUHandler.ExecuteCommand(passwd))
                ; // UNDONE?
        }
        else
            unlink(it->first.c_str());
        percent += (100.0f/float(pApp->FileSums.size()));
        UpFunc((int)percent, it->first, pData);
    }
    
    RemoveFromRegister(pApp);
    
    return UNINST_SUCCESS;
}

void CRegister::GetRegisterEntries(std::vector<app_entry_s *> *AppVec)
{
    struct dirent *dirstruct;
    struct stat filestat;
    DIR *dp = opendir(GetAppRegDir());

    if (!dp)
        return;

    char curdir[1024];
    if (!getcwd(curdir, sizeof(curdir))) throwerror(true, "Could not read current directory");
    
    // Changing directory temporary makes things easier
    if (chdir(GetAppRegDir()) != 0)
        throwerror(true, "Could not open directory '%s'", GetAppRegDir());
        
    while ((dirstruct = readdir(dp)) != 0)
    {
        if (lstat(dirstruct->d_name, &filestat) != 0)
        {
            debugline("dir err: %s\n", strerror(errno));
            continue;
        }
        
        if (!(filestat.st_mode & S_IFDIR))
            continue; // Has to be a directory...

        if (!strcmp(dirstruct->d_name, "."))
            continue;

        if (!strcmp(dirstruct->d_name, ".."))
            continue;
        
        app_entry_s *pApp = GetAppEntry(dirstruct->d_name);
        if (pApp)
            AppVec->push_back(pApp);
    }

    closedir (dp);
    
    chdir(curdir); // Return back to original directory
}

void CRegister::CalcSums()
{
    std::ofstream outfile(GetSumListFile(InstallInfo.program_name.c_str()));
    
    if (!outfile)
        return; // UNDONE
    
    WriteSums("plist_extrpath", outfile, NULL);
    WriteSums(CreateText("plist_extrpath_%s", InstallInfo.os.c_str()), outfile, NULL);
    WriteSums(CreateText("plist_extrpath_%s_%s", InstallInfo.os.c_str(), InstallInfo.cpuarch.c_str()), outfile, NULL);
    
    for (std::list<command_entry_s *>::iterator it=InstallInfo.command_entries.begin(); it!=InstallInfo.command_entries.end();
         it++)
    {
        if ((*it)->parameter_entries.empty()) continue;
        for (std::map<std::string, param_entry_s *>::iterator it2=(*it)->parameter_entries.begin();
             it2!=(*it)->parameter_entries.end(); it2++)
        {
            if (it2->second->varname.empty())
                continue;
            
            WriteSums(CreateText("plist_var_%s", it2->second->varname.c_str()), outfile, &it2->second->value);
            WriteSums(CreateText("plist_var_%s_%s", it2->second->varname.c_str(), InstallInfo.os.c_str()), outfile,
                      &it2->second->value);
            WriteSums(CreateText("plist_var_%s_%s", it2->second->varname.c_str(), InstallInfo.os.c_str(),
                      InstallInfo.cpuarch.c_str()), outfile, &it2->second->value);
        }
    }
}

bool CRegister::CheckSums(const char *progname)
{
    std::ifstream file(GetSumListFile(progname));
    
    if (!file)
        return true;
    
    app_entry_s *pApp = GetAppEntry(progname);
    if (!pApp)
        return true;
    
    for (std::map<std::string, std::string>::iterator it=pApp->FileSums.begin(); it!=pApp->FileSums.end(); it++)
    {
        if (!FileExists(it->first))
        {
            debugline("Couldn't find file %s\n", it->first.c_str());
            continue;
        }
        
        if (GetMD5(it->first) != it->second)
        {
            debugline("MD5 Mismatch: %s for %s\n", it->first.c_str(), progname);
            return false;
        }
    }
    
    return true;
}
