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
    // Init all archives (if file doesn't exist nothing will be done)
    InitArchive(CreateText("%s/instarchive_all", m_InstallInfo.own_dir.c_str()));
    InitArchive(CreateText("%s/instarchive_all_%s", m_InstallInfo.own_dir.c_str(), m_InstallInfo.cpuarch.c_str()));
    InitArchive(CreateText("%s/instarchive_%s", m_InstallInfo.own_dir.c_str(), m_InstallInfo.os.c_str()));
    InitArchive(CreateText("%s/instarchive_%s_%s", m_InstallInfo.own_dir.c_str(), m_InstallInfo.os.c_str(),
                m_InstallInfo.cpuarch.c_str()));
    
    if (m_ArchList.empty())
        return true; // No files to extract

    bool needroot = !WriteAccess(m_InstallInfo.dest_dir);
    
    if (needroot)
    {
        // Set up su
        m_SUHandler.SetUser("root");
        m_SUHandler.SetOutputFunc(ExtrSUOutFunc, this);
        m_SUHandler.SetTerminalOutput(false);
    }
    
    while (m_CurArchIter != m_ArchList.end())
    {
        m_szCurArchFName = m_CurArchIter->first;
        
        // Set extract command
        std::string command = "cat " + std::string(m_szCurArchFName);

        if (m_InstallInfo.archive_type == ARCH_GZIP)
            command += " | gzip -cd | tar xvf -";
        else if (m_InstallInfo.archive_type == ARCH_BZIP2)
            command += " | bzip2 -d | tar xvf -";

        if (needroot)
        {
            m_SUHandler.SetCommand(command);
            /*if (!m_SUHandler.ExecuteCommand(passwd))
            return false;UNDONE*/
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
                UpdateStatus(line);
        }
        m_CurArchIter++;
    }
    return true;
}

bool CBaseInstall::Install(void)
{
    if ((InstallInfo.dest_dir_type == DEST_SELECT) || (InstallInfo.dest_dir_type == DEST_DEFAULT))
    {
        if (!ChoiceBox(CreateText(GetTranslation("This will install %s to the following directory:\n%s\nContinue?"),
             InstallInfo.program_name.c_str(), MakeCString(InstallInfo.dest_dir)), GetTranslation("Exit program"),
             GetTranslation("Continue"), NULL))
            EndProg();
    }
    else
    {
        if (!ChoiceBox(CreateText(GetTranslation("This will install %s\nContinue?"), InstallInfo.program_name.c_str()),
             GetTranslation("Exit program"), GetTranslation("Continue"), NULL))
            EndProg();
    }
        
    // Check if we need root access
    bool askpass = false;
    for (std::list<command_entry_s *>::iterator it=InstallInfo.command_entries.begin();
         it!=InstallInfo.command_entries.end(); it++)
    {
        if ((*it)->need_root != NO_ROOT)
        {
            // Command may need root permission, check if it is so
            if ((*it)->need_root == DEPENDED_ROOT)
            {
                param_entry_s *p = GetParamByVar((*it)->dep_param);
                if (p && !WriteAccess(p->value))
                {
                    (*it)->need_root = NEED_ROOT;
                    if (!askpass) askpass = true;
                }
            }
            else if (!askpass) askpass = true;
        }
    }

    if (!askpass)
        askpass = !WriteAccess(InstallInfo.dest_dir);

    m_SUHandler.SetOutputFunc(ExtrSUOutFunc, this);
    m_SUHandler.SetUser("root");
    m_SUHandler.SetTerminalOutput(false);

    if (askpass && m_SUHandler.NeedPassword())
    {
        while(true)
        {
            CleanPasswdString(m_szPassword);
            
            m_szPassword = GetPassword();
            
            // Check if password is invalid
            if (!m_szPassword)
            {
                if (ChoiceBox(GetTranslation("Root access is required to continue\nAbort installation?"),
                    GetTranslation("No"), GetTranslation("Yes"), NULL))
                    EndProg();
            }
            else
            {
                if (m_SUHandler.TestSU(m_szPassword))
                    break;

                // Some error appeared
                if (m_SUHandler.GetError() == LIBSU::CLibSU::SU_ERROR_INCORRECTPASS)
                    Warn(GetTranslation("Incorrect password given for root user\nPlease retype"));
                else
                {
                    throwerror(true, GetTranslation("Could not use su to gain root access"
                            "Make sure you can use su(adding the current user to the wheel group may help"));
                }
            }
        }
    }
    
    if (chdir(InstallInfo.dest_dir.c_str()))
        throwerror(true, "Could not open directory '%s'", InstallInfo.dest_dir.c_str());
    
//    Install();
    return true;
}

void CBaseInstall::UpdateStatus(const char *s)
{
    if (!s || !s[0])
        return;
    
    std::string stat = s;
                
    if (stat.compare(0, 2, "x ") == 0)
        stat.erase(0, 2);
                               
    EatWhite(stat);

    AddStatusText("Extracting file: " + stat);
                
    stat += '\n'; // File names are stored with a newline
                
    m_fExtrPercent += ((float)m_ArchList[m_szCurArchFName].filesizes[stat]/(float)m_iTotalArchSize)*100.0f;
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
                    if (strstrm >> m_InstallInfo.dest_dir)
                        m_InstallInfo.dest_dir_type = DEST_DEFAULT;
                }
            }
            else if (str == "languages")
            {
                std::string lang;
                while (strstrm >> lang)
                    m_InstallInfo.languages.push_back(lang);
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
    debugline("installdir: %s\n", m_InstallInfo.dest_dir.c_str());
    debugline("dir type: %d\n", m_InstallInfo.dest_dir_type);
    debugline("languages: ");
    for (std::list<std::string>::iterator it=m_InstallInfo.languages.begin(); it!=m_InstallInfo.languages.end(); it++)
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
