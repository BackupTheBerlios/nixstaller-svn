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
        
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>

#include "main.h"

const char *CBaseAppManager::GetSumListFile(const char *progname)
{
    const char *dir = CreateText("%s/%s", GetAppRegDir(), progname);
    if (mkdir(dir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
        ThrowError(false, "Could not create nixstaller app-config directory!(%s)", strerror(errno));
    return CreateText("%s/list", dir);
}

app_entry_s *CBaseAppManager::GetAppEntry(const char *progname)
{
    const char *filename = GetRegConfFile(progname);
    if (!FileExists(filename))
        return NULL;

    app_entry_s *pAppEntry = new app_entry_s;
    pAppEntry->name = progname;
    
    std::ifstream file(filename);
    std::string str, field;
    
    while(file && (file >> str))
    {
        field = ReadRegField(file);
        
        if (str == "version")
            pAppEntry->version = field;
        else if (str == "description")
            pAppEntry->description = field;
        else if (str == "url")
            pAppEntry->url = field;
    }

    file.close();

    filename = GetSumListFile(progname);
    if (FileExists(filename))
    {
        std::string line, sum;

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

void CBaseAppManager::RemoveFromRegister(app_entry_s *pApp)
{
    if (pApp->name.empty())
        return; // Otherwise we delete the main config directory ;)
        
    system(CreateText("rm -rf %s/%s", GetAppRegDir(), pApp->name.c_str())); // Lazy way to remove whole dir   
}

void CBaseAppManager::Uninstall(app_entry_s *pApp, bool checksum)
{
    float percent = 0.0f;
    char *passwd = NULL;
    std::map<std::string, std::string>::iterator it;
    bool needroot = false;
    
    // Check if we got permission to remove all files
    for (it=pApp->FileSums.begin(); it!=pApp->FileSums.end(); it++)
    {
        char *fname = StrDup(it->first.c_str());
        char *dname = dirname(CreateText(fname));
        if (dname && dname[0] && (!WriteAccess(dname) || !ReadAccess(dname)))
        {
            needroot = true;
            free(fname);
            break;
        }
        free(fname);
    }
    
    if (needroot)
        SetUpSU("This uninstallation requires root(administrator) privileges in order to continue\n"
                "Please enter the password of the root user");
    
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
        
        SetProgress((int)percent);
        AddUninstOutput(it->first);
    }
    
    RemoveFromRegister(pApp);
}

void CBaseAppManager::GetRegisterEntries(std::vector<app_entry_s *> *AppVec)
{
    struct dirent *dirstruct;
    struct stat filestat;
    DIR *dp = opendir(GetAppRegDir());

    if (!dp)
        return;

    char curdir[1024];
    if (!getcwd(curdir, sizeof(curdir))) ThrowError(true, "Could not read current directory");
    
    // Changing directory temporary makes things easier
    if (chdir(GetAppRegDir()) != 0)
        ThrowError(true, "Could not open directory '%s'", GetAppRegDir());
        
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

bool CBaseAppManager::CheckSums(const char *progname)
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
#endif
