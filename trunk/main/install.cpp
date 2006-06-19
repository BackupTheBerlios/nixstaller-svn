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
#include <sys/wait.h>
#include "main.h"
#include <sys/utsname.h>

CBaseInstall::~CBaseInstall()
{
    if (!m_InstallInfo.command_entries.empty())
    {
        std::list<command_entry_s *>::iterator p = m_InstallInfo.command_entries.begin();
        for(;p!=m_InstallInfo.command_entries.end();p++)
        {
            if (!(*p)->parameter_entries.empty())
            {
                for (std::map<std::string, param_entry_s *>::iterator p2=(*p)->parameter_entries.begin();
                     p2!=(*p)->parameter_entries.end();p2++)
                    delete (*p2).second;
            }
            delete *p;
        }
    }
}

bool CBaseInstall::Init()
{   
    // Get current OS and cpu arch name
    struct utsname inf;
    if (uname(&inf) == -1)
        return false;
    
    m_szOS = inf.sysname;
    std::transform(m_szOS.begin(), m_szOS.end(), m_szOS.begin(), tolower);

    m_szCPUArch = inf.machine;
    // Convert iX86 to x86
    if ((m_szCPUArch[0] == 'i') && (m_szCPUArch.compare(2, 2, "86") == 0))
        m_szCPUArch = "x86";

    char curdir[1024];
    if (getcwd(curdir, sizeof(curdir)) == 0)
        ThrowError(false, "Could not read current directory");

    m_szOwnDir = curdir;
    debugline("Current dir: %s\n", m_szOwnDir.c_str());
    
    if (!CMain::Init()) // Init main, will also read config files
        return false;
    
    if (m_InstallInfo.dest_dir_type == DEST_TEMP)
        m_szDestDir = curdir;
    else if (m_InstallInfo.dest_dir_type == DEST_SELECT)
    {
        const char *env = getenv("HOME");
        if (env) m_szDestDir = env;
        else m_szDestDir = "/";
    }
    
    // Check if destination directory is readable
    if ((m_InstallInfo.dest_dir_type == DEST_DEFAULT) && !ReadAccess(m_szDestDir))
        ThrowError(true, CreateText("This installer will install files to the following directory:\n%s\n"
                                    "However you don't have read permissions to this directory\n"
                                    "Please restart the installer as a user who does or as the root user",
                   m_szDestDir.c_str()));

    return true;
}

void CBaseInstall::SetNextStep()
{
    m_sCurrentStep++;
    m_fInstallProgress += (1.0f/(float)m_sInstallSteps)*100.0f;
    SetProgress(m_fInstallProgress);
}

void CBaseInstall::InitArchive(char *archname)
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
        m_ArchList[archname].filesizes[arfilename] = size;
        m_iTotalArchSize += size;
    }
    
    if (!m_szCurArchFName)
    {
        m_szCurArchFName = archname;
        m_CurArchIter = m_ArchList.begin();
    }
}

void CBaseInstall::ExtractFiles()
{    
    if (m_ArchList.empty())
        return; // No files to extract

    ChangeStatusText("Extracting files", m_sCurrentStep, m_sInstallSteps);
    
    m_bAlwaysRoot = !WriteAccess(m_szDestDir);
    
    if (m_bAlwaysRoot)
        m_SUHandler.SetOutputFunc(ExtrSUOutFunc, this);
    
    while (m_CurArchIter != m_ArchList.end())
    {
        m_szCurArchFName = m_CurArchIter->first;
        
        // Set extract command
        std::string command = "cat " + std::string(m_szCurArchFName);

        if (m_InstallInfo.archive_type == ARCH_GZIP)
            command += " | gzip -cd | tar xvf -";
        else if (m_InstallInfo.archive_type == ARCH_BZIP2)
            command += " | bzip2 -d | tar xvf -";

        if (m_bAlwaysRoot)
        {
            m_SUHandler.SetCommand(command);
            if (!m_SUHandler.ExecuteCommand(m_szPassword))
            {
                CleanPasswdString(m_szPassword);
                m_szPassword = NULL;
                ThrowError(true, "Error during extracting files");
            }
        }
        else
        {
            command += " 2>&1"; // tar may output files to stderr
            
            FILE *pipe = popen(command.c_str(), "r");
            if (!pipe)
                ThrowError(true, "Error during extracting files (could not open pipe)");
            
            char line[512];
            while (fgets(line, sizeof(line), pipe))
                UpdateStatus(line);
            
            // Check if command exitted normally and close pipe
            int state = pclose(pipe);
            if (!WIFEXITED(state) || (WEXITSTATUS(state) == 127)) // SH returns 127 if command execution failes
                ThrowError(true, "Failed to execute install command (could not execute tar)");
        }
        m_CurArchIter++;
    }
    
    SetNextStep();
}

void CBaseInstall::ExecuteInstCommands(void)
{
    for (std::list<command_entry_s*>::iterator it=m_InstallInfo.command_entries.begin();
         it!=m_InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->command.empty()) continue;
        
        std::string command = (*it)->command + " " + GetParameters(*it);
    
        AddInstOutput(CreateText("\nExecute: %s\n\n", command.c_str()));
        ChangeStatusText((*it)->description.c_str(), m_sCurrentStep, m_sInstallSteps);

        if ((*it)->need_root == NEED_ROOT || m_bAlwaysRoot)
        {
            m_SUHandler.SetPath((*it)->path.c_str());
            m_SUHandler.SetCommand(command);
            if (!m_SUHandler.ExecuteCommand(m_szPassword))
            {
                if ((*it)->exit_on_failure)
                {
                    CleanPasswdString(m_szPassword);
                    m_szPassword = NULL;
                    ThrowError(true, "%s\n('%s')", GetTranslation("Failed to execute install command"),
                               m_SUHandler.GetErrorMsgC());
                }
            }
        }
        else
        {
            // Redirect stderr to stdout, so that errors will be displayed too
            command += " 2>&1";
            
            setenv("PATH", (*it)->path.c_str(), 1);
            FILE *pPipe = popen(command.c_str(), "r");
            if (pPipe)
            {
                char buf[1024];
                while(fgets(buf, sizeof(buf), pPipe))
                    AddInstOutput(buf);
                
                // Check if command exitted normally and close pipe
                int state = pclose(pPipe);
                if (!WIFEXITED(state) || (WEXITSTATUS(state) == 127)) // SH returns 127 if command execution failes
                {
                    if ((*it)->exit_on_failure)
                    {
                        CleanPasswdString(m_szPassword);
                        m_szPassword = NULL;
                        ThrowError(true, "Failed to execute install command");
                    }
                }
            }
            else
            {
                CleanPasswdString(m_szPassword);
                m_szPassword = NULL;
                ThrowError(true, "Could not execute installation commands (could not open pipe)");
            }
        }
        
        SetNextStep();
    }
}

void CBaseInstall::Install(void)
{
    if ((m_InstallInfo.dest_dir_type == DEST_SELECT) || (m_InstallInfo.dest_dir_type == DEST_DEFAULT))
    {
        if (!ChoiceBox(CreateText(GetTranslation("This will install %s to the following directory:\n%s\nContinue?"),
             m_InstallInfo.program_name.c_str(), MakeCString(m_szDestDir)), GetTranslation("Exit program"),
             GetTranslation("Continue"), NULL))
            EndProg();
    }
    else
    {
        if (!ChoiceBox(CreateText(GetTranslation("This will install %s\nContinue?"), m_InstallInfo.program_name.c_str()),
             GetTranslation("Exit program"), GetTranslation("Continue"), NULL))
            EndProg();
    }
        
    // Init all archives (if file doesn't exist nothing will be done)
    InitArchive(CreateText("%s/instarchive_all", m_szOwnDir.c_str()));
    InitArchive(CreateText("%s/instarchive_all_%s", m_szOwnDir.c_str(), m_szCPUArch.c_str()));
    InitArchive(CreateText("%s/instarchive_%s", m_szOwnDir.c_str(), m_szOS.c_str()));
    InitArchive(CreateText("%s/instarchive_%s_%s", m_szOwnDir.c_str(), m_szOS.c_str(),
                m_szCPUArch.c_str()));

    // Count all install steps that have to be taken
    m_sInstallSteps = m_sCurrentStep = !m_ArchList.empty(); // Extracting is one step
    m_sInstallSteps += m_InstallInfo.command_entries.size(); // Every install command is one step
    
    // Check if we need root access
    bool needroot = false;
    for (std::list<command_entry_s *>::iterator it=m_InstallInfo.command_entries.begin();
         it!=m_InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->need_root != NO_ROOT)
        {
            // Command may need root permission, check if it is so
            if ((*it)->need_root == DEPENDED_ROOT)
            {
                param_entry_s *p = GetParamByVar((*it)->dep_param);
                if (p && FileExists(p->value) && !WriteAccess(p->value))
                {
                    (*it)->need_root = NEED_ROOT;
                    if (!needroot) needroot = true;
                }
            }
            else if (!needroot) needroot = true;
        }
    }

    if (!needroot)
        needroot = (FileExists(m_szDestDir) && !WriteAccess(m_szDestDir));

    if (needroot)
        SetUpSU("This installation requires root(administrator) privileges in order to continue\n"
                "Please enter the password of the root user");
    
    if (chdir(m_szDestDir.c_str())) 
        ThrowError(true, "Could not open directory '%s'", m_szDestDir.c_str());
    
    ExtractFiles();
    ExecuteInstCommands();
    
    AddInstOutput("Registering installation...");
    RegisterInstall();
    CalcSums();
    //Register.CheckSums(m_InstallInfo.program_name.c_str());
    AddInstOutput("done\n");

    SetProgress(100);
    MsgBox(GetTranslation("Installation of %s complete!"), m_InstallInfo.program_name.c_str());
    CleanPasswdString(m_szPassword);
    m_szPassword = NULL;
}

void CBaseInstall::UpdateStatus(const char *s)
{
    if (!s || !s[0])
        return;
    
    std::string stat = s;
                
    if (stat.compare(0, 2, "x ") == 0)
        stat.erase(0, 2);
                               
    EatWhite(stat);

    if (m_ArchList[m_szCurArchFName].filesizes.find(stat) == m_ArchList[m_szCurArchFName].filesizes.end())
    {
        debugline("Couldn't find %s\n", stat.c_str());
        return;
    }
    
    m_fExtrPercent += ((float)m_ArchList[m_szCurArchFName].filesizes[stat]/(float)m_iTotalArchSize)*100.0f;

    AddInstOutput("Extracting file: " + stat + '\n');
    SetProgress(m_fExtrPercent/(float)m_sInstallSteps);
}

bool CBaseInstall::ReadConfig()
{
    const int maxread = std::numeric_limits<std::streamsize>::max();
    std::ifstream file("config/install.cfg");
    std::string str, line, tmp, ParamName;
    bool incommandentry = false, inparamentry = false;
    command_entry_s *pCommandEntry = NULL;
    param_entry_s *pParamEntry = NULL;
    
    while(file)
    {
        // Read one line at a time...this makes sure that we don't start reading at a new line if the user 'forgot'
        // to enter a value after the first command/variabele
        if (!std::getline(file, line))
            break;
        
        std::istringstream strstrm(line); // Use a stringstream to easily read from this line
        
        if (!(strstrm >> str))
            continue;

        if (str[0] == '#') // Comment, skip
            continue;

        if (incommandentry)
        {
            if (inparamentry)
            {
                if (str[0] == ']')
                {
                    if (!ParamName.empty())
                        pCommandEntry->parameter_entries[ParamName] = pParamEntry;
                    pParamEntry = NULL;
                    inparamentry = false;
                }
                else if (str == "name")
                {
                    std::getline(strstrm, tmp);
                    ParamName = EatWhite(tmp);
                }
                else if (str == "parameter")
                {
                    std::getline(strstrm, tmp);
                    pParamEntry->parameter = EatWhite(tmp);
                }
                else if (str == "description")
                {
                    std::getline(strstrm, tmp);
                    EatWhite(tmp);
                    if (tmp[0] == '[')
                        GetTextFromBlock(file, tmp);
                    pParamEntry->description = tmp;
                }
                else if (str == "defaultval")
                {
                    std::getline(strstrm, tmp);
                    pParamEntry->defaultval = pParamEntry->value = EatWhite(tmp);
                }
                else if (str == "varname")
                {
                    std::getline(strstrm, tmp);
                    pParamEntry->varname = EatWhite(tmp);
                }
                else if (str == "addchoice")
                {
                    std::getline(strstrm, tmp);
                    pParamEntry->options.push_back(EatWhite(tmp));
                }
                else if (str == "type")
                {
                    std::string type;
                    strstrm >> type;
                    if (type == "string")
                        pParamEntry->param_type = PTYPE_STRING;
                    else if (type == "dir")
                        pParamEntry->param_type = PTYPE_DIR;
                    else if (type == "list")
                        pParamEntry->param_type = PTYPE_LIST;
                    else if (type == "bool")
                        pParamEntry->param_type = PTYPE_BOOL;
                }
            }
            else
            {
                if (str[0] == ']')
                {
                    m_InstallInfo.command_entries.push_back(pCommandEntry);
                    pCommandEntry = NULL;
                    incommandentry = false;
                }
                else if (str == "command")
                {
                    std::getline(strstrm, tmp);
                    EatWhite(tmp);
                    if (tmp[0] == '[')
                        GetTextFromBlock(file, tmp);
                    pCommandEntry->command = tmp;
                }
                else if (str == "description")
                {
                    std::getline(strstrm, tmp);
                    pCommandEntry->description = EatWhite(tmp);
                }
                else if (str == "needroot")
                {
                    std::string s;
                    strstrm >> s;
                    if (s == "true")
                        pCommandEntry->need_root = NEED_ROOT;
                    else if (s == "false")
                        pCommandEntry->need_root = NO_ROOT;
                    else if (s == "dependson")
                    {
                        if (strstrm >> pCommandEntry->dep_param)
                            pCommandEntry->need_root = DEPENDED_ROOT;
                    }
                }
                else if (str == "exitonfailure")
                {
                    std::string ex;
                    strstrm >> ex;
                    if (ex == "true")
                        pCommandEntry->exit_on_failure = true;
                    else if (ex == "false")
                        pCommandEntry->exit_on_failure = false;
                }
                else if (str == "path")
                {
                    std::getline(strstrm, tmp);
                    pCommandEntry->path = EatWhite(tmp);

                }
                else if (str == "addparam")
                {
                    file.ignore(maxread, '[');
                    inparamentry = true;
                    pParamEntry = new param_entry_s;
                    ParamName.clear();
                }
            }
        }
        else
        {
            if (str == "appname")
            {
                std::getline(strstrm, tmp);
                m_InstallInfo.program_name = EatWhite(tmp);

            }
            else if (str == "version")
            {
                std::getline(strstrm, tmp);
                m_InstallInfo.version = EatWhite(tmp);
            }
            else if (str == "url")
            {
                std::getline(strstrm, tmp);
                m_InstallInfo.url = EatWhite(tmp);
            }
            else if (str == "description")
            {
                std::getline(strstrm, tmp);
                EatWhite(tmp);
                if (tmp[0] == '[')
                    GetTextFromBlock(file, tmp);
                m_InstallInfo.description = tmp;
            }
            else if (str == "archtype")
            {
                std::string type;
                strstrm >> type;
                if (type == "gzip")
                    m_InstallInfo.archive_type = ARCH_GZIP;
                else if (type == "bzip2")
                    m_InstallInfo.archive_type = ARCH_BZIP2;
            }
            else if (str == "installdir")
            {
                std::string type;
                strstrm >> type;
                if (type == "select")
                    m_InstallInfo.dest_dir_type = DEST_SELECT;
                if (type == "temp")
                    m_InstallInfo.dest_dir_type = DEST_TEMP;
                else if (type == "default")
                {
                    if (strstrm >> m_szDestDir)
                        m_InstallInfo.dest_dir_type = DEST_DEFAULT;
                }
            }
            else if (str == "languages")
            {
                std::string lang;
                while (strstrm >> lang)
                    m_Languages.push_back(lang);
            }
            else if (str == "intropic")
            {
                std::getline(strstrm, tmp);
                m_InstallInfo.intropicname = EatWhite(tmp);
            }
            else if (str == "commandentry")
            {
                file.ignore(maxread, '[');
                incommandentry = true;
                pCommandEntry = new command_entry_s;
            }
        }
    }

#ifndef RELEASE

    debugline("appname: %s\n", m_InstallInfo.program_name.c_str());
    debugline("version: %s\n", m_InstallInfo.version.c_str());
    debugline("archtype: %d\n", m_InstallInfo.archive_type);
    debugline("installdir: %s\n", m_szDestDir.c_str());
    debugline("dir type: %d\n", m_InstallInfo.dest_dir_type);
    debugline("languages: ");
    for (std::list<std::string>::iterator it=m_Languages.begin(); it!=m_Languages.end(); it++)
        debugline("%s ", it->c_str());
    debugline("\n");

    debugline("Comp entries:\n");
    for (std::list<command_entry_s *>::iterator p=m_InstallInfo.command_entries.begin();p!=m_InstallInfo.command_entries.end();p++)
    {
        debugline("Need root: %d\n", (*p)->need_root);
        debugline("Command: %s\n", (*p)->command.c_str());
        debugline("Description: %s\n", (*p)->description.c_str());
        debugline("Depends on param: %s\n", (*p)->dep_param.c_str());
        debugline("Exit on failure: %d\n", (*p)->exit_on_failure);
        debugline("Params:\n");
        for (std::map<std::string, param_entry_s *>::iterator
             p2=(*p)->parameter_entries.begin();p2!=(*p)->parameter_entries.end();p2++)
        {
            debugline("\tName: %s\n\tType: %d\n\tParameter: %s\n\tDefault: %s\n\t"
                    "Description: %s\n\tVarname: %s\n", (*p2).first.c_str(), (*p2).second->param_type,
            (*p2).second->parameter.c_str(),
            (*p2).second->defaultval.c_str(), (*p2).second->description.c_str(),
            (*p2).second->varname.c_str());
            debugline("\tOptions: ");
            for (std::list<std::string>::iterator p3=(*p2).second->options.begin();p3!=(*p2).second->options.end();p3++)
                debugline("%s ", p3->c_str());
            debugline("\n");
        }
    }

#endif

    return true;
}

void CBaseInstall::WriteSums(const char *filename, std::ofstream &outfile, const std::string *var)
{
    std::ifstream infile(filename);
    std::string line;
    while (infile && std::getline(infile, line))
    {
        std::string dir;
        if (var)
            dir = *var;
        else
            dir = m_szDestDir;
        
        if (dir[dir.length()-1] != '/')
            dir += '/';

        line = dir + line;
        outfile << GetMD5(line) << " " << line << "\n";
    }
}

void CBaseInstall::WriteRegEntry(const char *entry, const std::string &field, std::ofstream &file)
{
    file << entry << ' ';
    
    std::string format = field;
    std::string::size_type index = 0;
    
    while ((index = format.find("\"", index+2)) != std::string::npos)
        format.replace(index, 1, "\\\"");
    
    file << '\"' << format << "\"\n";
}

void CBaseInstall::CalcSums()
{
    std::ofstream outfile(GetSumListFile(m_InstallInfo.program_name.c_str()));
    const char *dir = m_szOwnDir.c_str();
    
    if (!outfile)
        return; // UNDONE
    
    WriteSums(CreateText("%s/plist_extrpath", dir), outfile, NULL);
    WriteSums(CreateText("%s/plist_extrpath_%s", dir, m_szOS.c_str()), outfile, NULL);
    WriteSums(CreateText("%s/plist_extrpath_%s_%s", dir, m_szOS.c_str(), m_szCPUArch.c_str()), outfile, NULL);
    
    for (std::list<command_entry_s *>::iterator it=m_InstallInfo.command_entries.begin();
         it!=m_InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->parameter_entries.empty()) continue;
        for (std::map<std::string, param_entry_s *>::iterator it2=(*it)->parameter_entries.begin();
             it2!=(*it)->parameter_entries.end(); it2++)
        {
            if (it2->second->varname.empty())
                continue;
            
            WriteSums(CreateText("%s/plist_var_%s", dir, it2->second->varname.c_str()), outfile, &it2->second->value);
            WriteSums(CreateText("%s/plist_var_%s_%s", dir, it2->second->varname.c_str(), m_szOS.c_str()), outfile,
                      &it2->second->value);
            WriteSums(CreateText("%s/plist_var_%s_%s", dir, it2->second->varname.c_str(), m_szOS.c_str(),
                      m_szCPUArch.c_str()), outfile, &it2->second->value);
        }
    }
}

bool CBaseInstall::IsInstalled(bool checkver)
{
    if (!FileExists(GetRegConfFile(m_InstallInfo.program_name.c_str())))
        return false;
    
    app_entry_s *pApp = GetAppRegEntry(m_InstallInfo.program_name.c_str());
    
    if (!pApp)
        return false;
    
    return (!checkver || (pApp->version == m_InstallInfo.version));
}

void CBaseInstall::RegisterInstall(void)
{
    /*if (IsInstalled(true))
        return; UNDONE? */
    
    std::ofstream file(GetRegConfFile(m_InstallInfo.program_name.c_str()));
    
    if (!file)
        ThrowError(false, "Error while opening register file");

    WriteRegEntry("regver", m_szRegVer, file);
    WriteRegEntry("version", m_InstallInfo.version, file);
    WriteRegEntry("url", m_InstallInfo.url, file);
    WriteRegEntry("description", m_InstallInfo.description, file);
}

param_entry_s *CBaseInstall::GetParamByName(std::string str)
{
    for (std::list<command_entry_s *>::iterator it=m_InstallInfo.command_entries.begin();
         it!=m_InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->parameter_entries.empty()) continue;
        return ((*it)->parameter_entries.find(str))->second;
    }
    return NULL;
}

param_entry_s *CBaseInstall::GetParamByVar(std::string str)
{
    for (std::list<command_entry_s *>::iterator it=m_InstallInfo.command_entries.begin();
         it!=m_InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->parameter_entries.empty()) continue;
        for (std::map<std::string, param_entry_s *>::iterator it2=(*it)->parameter_entries.begin();
             it2!=(*it)->parameter_entries.end(); it2++)
        {
            if (it2->second->varname == str)
                return it2->second;
        }
    }
    return NULL;
}

const char *CBaseInstall::GetParamDefault(param_entry_s *pParam)
{
    if (pParam->param_type == PTYPE_BOOL)
    {
        if (pParam->defaultval == "true")
            return GetTranslation("Enabled");
        else
            return GetTranslation("Disabled");
    }
    return GetTranslation(pParam->defaultval.c_str());
}

const char *CBaseInstall::GetParamValue(param_entry_s *pParam)
{
    if (pParam->param_type == PTYPE_BOOL)
    {
        if (pParam->value == "true")
            return GetTranslation("Enabled");
        else
            return GetTranslation("Disabled");
    }
    return GetTranslation(pParam->value.c_str());
}

std::string CBaseInstall::GetParameters(command_entry_s *pCommandEntry)
{
    std::string args, param;
    std::string::size_type pos;

    for(std::map<std::string, param_entry_s *>::iterator it=pCommandEntry->parameter_entries.begin();
        it!=pCommandEntry->parameter_entries.end();it++)
    {
        switch (it->second->param_type)
        {
            case PTYPE_STRING:
            case PTYPE_DIR:
            case PTYPE_LIST:
                param = it->second->parameter;
                pos = param.find("%s");
                if (pos != std::string::npos)
                    param.replace(pos, 2, it->second->value);
                args += " " + param;
                break;
            case PTYPE_BOOL:
                if (it->second->value == "true") args += " " + it->second->parameter;
                break;
        }
    }
    return args;
}
