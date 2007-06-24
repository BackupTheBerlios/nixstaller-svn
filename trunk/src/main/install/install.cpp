/*
    Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)

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

#include <fstream>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <poll.h>
#include <errno.h>

#include "main/main.h"
#include "main/install/basescreen.h"
#include "main/install/luacfgmenu.h"
#include "main/install/luacheckbox.h"
#include "main/install/luadirselector.h"
#include "main/install/luagroup.h"
#include "main/install/luaradiobutton.h"
#include "main/install/luainput.h"
#include "main/install/luamenu.h"
#include "main/install/luaprogressbar.h"
#include "main/install/luatextfield.h"
#include "main/lua/lua.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "main/lua/luatable.h"

// -------------------------------------
// Base Installer Class
// -------------------------------------

void CBaseInstall::Init(int argc, char **argv)
{   
    m_szBinDir = dirname(argv[0]);
    
    CMain::Init(argc, argv); // Init main, will also read config files
    
    // Obtain install variabeles from lua
    NLua::LuaGet(m_InstallInfo.program_name, "appname", "cfg");
    NLua::LuaGet(m_InstallInfo.intropicname, "intropic", "cfg");
    
    NLua::LuaGet(m_InstallInfo.archive_type, "archivetype", "cfg");
    if ((m_InstallInfo.archive_type != "gzip") && (m_InstallInfo.archive_type != "bzip2") &&
        (m_InstallInfo.archive_type != "lzma"))
        throw Exceptions::CExLua("Wrong archivetype specified! Should be gzip, bzip2 or lzma.");
}

void CBaseInstall::InitArchive(char *archname)
{
    if (!FileExists(archname))
    {
        debugline("InitArchive: No such file: %s\n", archname);
        return;
    }
        
    char *fname = CreateText("%s.sizes", archname);
    std::ifstream file(fname);
    std::string arfilename;
    unsigned long size;

    // Read first column to size and the other column(s) to arfilename
    while(file && (file >> size) && std::getline(file, arfilename))
    {
        EatWhite(arfilename);
        m_ArchList[archname].filesizes[arfilename] = size;
        m_ulTotalArchSize += size;
    }
}

void CBaseInstall::ExtractFiles()
{
    VerifyIfInstalling();
    
    if (m_ArchList.empty())
    {
        debugline("No files to extract\n");
        return; // No files to extract
    }
    
    UpdateStatusText("Extracting files");
    
    m_bAlwaysRoot = !WriteAccess(GetDestDir());
    
    if (m_bAlwaysRoot)
        m_SUHandler.SetOutputFunc(ExtrSUOutFunc, this);

    for (TArchType::iterator it=m_ArchList.begin(); it!=m_ArchList.end(); it++)
    {
        m_szCurArchFName = it->first;
        
        // Set extract command
        std::string command = "cat " + std::string(m_szCurArchFName);

        if (m_InstallInfo.archive_type == "gzip")
            command += " | gzip -cd | tar xvf -";
        else if (m_InstallInfo.archive_type == "bzip2")
            command += " | bzip2 -d | tar xvf -";
        else if (m_InstallInfo.archive_type == "lzma")
        {
            command = "(" + m_szBinDir + "/../lzma-decode " + std::string(m_szCurArchFName) +
                    " arch.tar 2>&1 >/dev/null && tar xvf arch.tar && rm arch.tar)";
        }
        
        debugline("Extr cmd: %s", command.c_str());
        
        if (m_bAlwaysRoot)
        {
            m_SUHandler.SetCommand(command);
            if (!m_SUHandler.ExecuteCommand(m_szPassword))
            {
                CleanPasswdString(m_szPassword);
                m_szPassword = NULL;
                throw Exceptions::CExCommand(command.c_str());
            }
        }
        else
        {
            command += " 2>&1"; // tar may output files to stderr
            
            CPipedCMD pipe(command.c_str());
            std::string line;
            
            while (pipe)
            {
                InstallThink();
                
                if (pipe.HasData())
                {
                    int ch = pipe.GetCh();
                    
                    if (ch != EOF)
                    {
                        line += (char)ch;
                        if ((char)ch == '\n')
                        {
                            UpdateExtrStatus(line.c_str());
                            line.clear();
                        }
                    }
                }
            }
            
            if (!line.empty())
                UpdateExtrStatus(line.c_str());
            
            pipe.Close(); // By calling Close() explicity its able to throw exceptions
        }
    }
}

void CBaseInstall::ExecuteCommand(const char *cmd, bool required, const char *path)
{
    VerifyIfInstalling();
    
    if (m_bAlwaysRoot)
    {
        ExecuteCommandAsRoot(cmd, required, path);
        return;
    }
    
    // Redirect stderr to stdout, so that errors will be displayed too
    const char *append = " 2>&1";
    char command[strlen(cmd) + strlen(append)];
    strcpy(command, cmd);
    strcat(command, append);
    
    if (path && *path)
        setenv("PATH", path, 1);
    else
        setenv("PATH", GetDefaultPath(), 1);
    
    AddInstOutput(CreateText("\nExecute: %s\n\n", cmd));
    
    CPipedCMD pipe(command);
    std::string line;
            
    while (pipe)
    {
        InstallThink();
                
        if (pipe.HasData())
        {
            int ch = pipe.GetCh();
                    
            if (ch != EOF)
            {
                line += (char)ch;
                if ((char)ch == '\n')
                {
                    AddInstOutput(line.c_str());
                    line.clear();
                }
            }
        }
    }
            
    if (!line.empty())
        AddInstOutput(line.c_str());
            
    if (required)
        pipe.Close(); // By calling Close() explicity its able to throw exceptions
}

void CBaseInstall::ExecuteCommandAsRoot(const char *cmd, bool required, const char *path)
{
    VerifyIfInstalling();
    
    if (!m_szPassword || !m_szPassword[0])
        SetUpSU("This installation requires root(administrator) privileges in order to continue\n"
                "Please enter the password of the root user");
    
    m_SUHandler.SetOutputFunc(CMDSUOutFunc, this);

    if (!path || !path[0])
        path = GetDefaultPath();
    
    m_SUHandler.SetPath(path);
    m_SUHandler.SetCommand(cmd);
    
    AddInstOutput(CreateText("\nExecute: %s\n\n", cmd));
    
    if (!m_SUHandler.ExecuteCommand(m_szPassword))
    {
        if (required)
        {
            CleanPasswdString(m_szPassword);
            m_szPassword = NULL;
            throw Exceptions::CExCommand(cmd);
        }
    }
}

void CBaseInstall::VerifyIfInstalling()
{
    if (!m_bInstalling)
        luaL_error(NLua::LuaState, "Error: function called when install is not in progress\n");
}

void CBaseInstall::UpdateStatusText(const char *msg)
{
    if (m_sCurrentStep > 0)
    {
        m_fInstallProgress += (1.0f/(float)m_sInstallSteps)*100.0f;
        SetProgress(SafeConvert<int>(m_fInstallProgress));
    }
    
    m_sCurrentStep++;
    
    if (m_sInstallSteps > 1)
        ChangeStatusText(CreateText("%s: %s (%d/%d)", GetTranslation("Status"),
                         GetTranslation(msg), m_sCurrentStep, m_sInstallSteps));
    else
        ChangeStatusText(CreateText("%s: %s", GetTranslation("Status"), GetTranslation(msg)));
}

void CBaseInstall::Install(void)
{
    if (!ChoiceBox(CreateText(GetTranslation("This will install %s\nContinue?"), m_InstallInfo.program_name.c_str()),
         GetTranslation("Exit program"), GetTranslation("Continue"), NULL))
        throw Exceptions::CExUser();
    
    m_bInstalling = true;
    
    // Init all archives (if file doesn't exist nothing will be done)
    InitArchive(CreateText("%s/instarchive_all", m_szOwnDir.c_str()));
    InitArchive(CreateText("%s/instarchive_all_%s", m_szOwnDir.c_str(), m_szCPUArch.c_str()));
    InitArchive(CreateText("%s/instarchive_%s_all", m_szOwnDir.c_str(), m_szOS.c_str()));
    InitArchive(CreateText("%s/instarchive_%s_%s", m_szOwnDir.c_str(), m_szOS.c_str(),
                m_szCPUArch.c_str()));

    std::string destdir = GetDestDir();
    if (FileExists(destdir))
    {
        if (!ReadAccess(destdir))
        {
            throw Exceptions::CExReadExtrDir(destdir.c_str());
/*            // Dirselector screens should also check for RO dirs, so this basicly only happens when the installer defaults to a single dir
            ThrowError(true, CreateText("This installer will install files to the following directory:\n%s\n"
                                        "However you don't have read permissions to this directory\n"
                                        "Please restart the installer as a user who does or as the root user", destdir));*/
        }
        else if (!WriteAccess(destdir))
        {
            SetUpSU("This installation requires root(administrator) privileges in order to continue\n"
                    "Please enter the password of the root user");
        }
    }
    
    CHDir(destdir);
    
    m_SUHandler.SetThinkFunc(SUThinkFunc, this);

    NLua::CLuaFunc func("Install");
    
    if (func)
        func(0);
    else
        ExtractFiles(); // Default behaviour
    
//     AddInstOutput("Registering installation...");
//     RegisterInstall();
//     CalcSums();
//     //Register.CheckSums(m_InstallInfo.program_name.c_str());
//     AddInstOutput("done\n");

    SetProgress(100);
    MsgBox(GetTranslation("Installation of %s complete!"), m_InstallInfo.program_name.c_str());
    CleanPasswdString(m_szPassword);
    m_szPassword = NULL;
    m_bInstalling = false;
}

void CBaseInstall::UpdateExtrStatus(const char *s)
{
    if (!s || !s[0])
        return;
    
    debugline("UpdateExtrStatus: %s\n", s);
    std::string stat = s;
                
    if (stat.compare(0, 2, "x ") == 0)
        stat.erase(0, 2);

    if (m_szOS == "sunos") // HACK: Solaris tar outputs additional information after file name, remove it
    {
        // Format is "x <filename>, <n> bytes, <n> tape blocks. The "x " part is already removed.
        
        // Search for second last comma
        TSTLStrSize commapos = stat.rfind(",");
        if (commapos != std::string::npos)
            commapos = stat.rfind(",", commapos-1);
        
        if (commapos != std::string::npos)
        {
            debugline("Results: %u and %u\n",(stat.find("bytes", commapos)), (stat.find("tape blocks", commapos)));
            // Unfortunaly file names can contain comma's so verify a little more...
            if ((stat.find("bytes", commapos) != std::string::npos) && (stat.find("tape blocks", commapos) != std::string::npos))
            {
                // Should be safe now...
                stat.erase(commapos);
            }
        }
    }
    
    EatWhite(stat);

    if (m_ArchList[m_szCurArchFName].filesizes.find(stat) == m_ArchList[m_szCurArchFName].filesizes.end())
    {
        debugline("Couldn't find %s\n", stat.c_str());
        return;
    }
    
    m_fExtrPercent += ((float)m_ArchList[m_szCurArchFName].filesizes[stat]/(float)m_ulTotalArchSize)*100.0f;

    AddInstOutput("Extracting file: " + stat + '\n');
    SetProgress(SafeConvert<int>(m_fExtrPercent/(float)m_sInstallSteps));
}

void CBaseInstall::SetDestDir(const char *dir)
{
    NLua::LuaSet(dir, "destdir", "install");
}

std::string CBaseInstall::GetDestDir(void)
{
    std::string ret;
    NLua::LuaGet(ret, "destdir", "install");
    return ret;
}

void CBaseInstall::InitLua()
{
    CMain::InitLua();
    
    CBaseScreen::LuaRegister();
    CBaseLuaGroup::LuaRegister();
    CBaseLuaInputField::LuaRegister();
    CBaseLuaCheckbox::LuaRegister();
    CBaseLuaRadioButton::LuaRegister();
    CBaseLuaDirSelector::LuaRegister();
    CBaseLuaCFGMenu::LuaRegister();
    CBaseLuaMenu::LuaRegister();
    CBaseLuaProgressBar::LuaRegister();
    CBaseLuaTextField::LuaRegister();

    NLua::RegisterFunction(LuaNewScreen, "newscreen", "install", this);
    NLua::RegisterFunction(LuaAddScreen, "addscreen", "install", this);
    NLua::RegisterFunction(LuaGetTempDir, "gettempdir", "install", this);
    NLua::RegisterFunction(LuaExtractFiles, "extractfiles", "install", this);
    NLua::RegisterFunction(LuaExecuteCMD, "execute", "install", this);
    NLua::RegisterFunction(LuaExecuteCMDAsRoot, "executeasroot", "install", this);
    NLua::RegisterFunction(LuaAskRootPW, "askrootpw", "install", this);
    NLua::RegisterFunction(LuaSetStatusMSG, "setstatus", "install", this);
    NLua::RegisterFunction(LuaSetStepCount, "setstepcount", "install", this);
    NLua::RegisterFunction(LuaPrintInstOutput, "print", "install", this);
    
    const char *env = getenv("HOME");
    if (env)
        SetDestDir(env);
    else
        SetDestDir("/");
    
    NLua::LoadFile("config/config.lua");
    NLua::LoadFile("install.lua");
    
/*    if (FileExists("config/run.lua"))
    {
        m_LuaVM.LoadFile("config/run.lua");
        if (m_LuaVM.InitCall("Init"))
            m_LuaVM.DoCall();
    }*/
    
    NLua::CLuaTable table("languages", "cfg");
    
    if (table)
    {
        int size = table.Size();
        std::string lang;
        for (int i=1; i<=size; i++)
        {
            table[i] >> lang;
            m_Languages.push_back(lang);
        }
    }
    
    table.Close();

#ifndef RELEASE
    debugline("appname: %s\n", m_InstallInfo.program_name.c_str());
    debugline("version: %s\n", m_InstallInfo.version.c_str());
    debugline("archtype: %s\n", m_InstallInfo.archive_type.c_str());
    debugline("installdir: %s\n", GetDestDir().c_str());
    debugline("languages: ");
    for (std::vector<std::string>::iterator it=m_Languages.begin(); it!=m_Languages.end(); it++)
        debugline("%s ", it->c_str());
    debugline("\n");
#endif
}

bool CBaseInstall::VerifyDestDir(void)
{
    std::string dir = GetDestDir();
    
    if (FileExists(dir))
    {
        if (!ReadAccess(dir))
        {
            MsgBox(GetTranslation("You don't have read permissions for this directory.\n"
                                  "Please choose another directory or rerun this program as a user who has read permissions."));
            return false;
        }
        if (!WriteAccess(dir))
        {
            return (ChoiceBox(GetTranslation("You don't have write permissions for this directory.\n"
                                             "The files can be extracted as the root user,\n"
                                             "but you'll need to enter the root password for this later."),
                    GetTranslation("Choose another directory"),
                    GetTranslation("Continue as root"), NULL) == 1);
        }
    }
    else
    {
        // Create directory?
        if (YesNoBox(CreateText(GetTranslation("Directory %s does not exist, do you want to create it?"), dir.c_str())))
        {
            try
            {
                MKDir(dir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH));
                return true;
            }
            catch(Exceptions::CExMKDir &e)
            {
                WarnBox("%s\n%s\n%s", GetTranslation("Could not create directory"), dir.c_str(), e.what());
            }
        }
        return false;
    }
    
    return true;
}

#ifdef WITH_APPMANAGER
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
            dir = GetDestDir();
        
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
/*    
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
    }*/
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
#endif

int CBaseInstall::LuaNewScreen(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *name = luaL_optstring(L, 1, "");
    NLua::CreateClass(pInstaller->CreateScreen(GetTranslation(name)), "screen");
    return 1;
}

int CBaseInstall::LuaAddScreen(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    pInstaller->AddScreen(1);
    lua_pop(NLua::LuaState, 1);
    return 0;
}

int CBaseInstall::LuaGetTempDir(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *ret = CreateText("%s/tmp", pInstaller->m_szOwnDir.c_str());
    
    if (!FileExists(ret))
        MKDir(ret, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH));
    
    lua_pushstring(L, ret);
    return 1;
}

int CBaseInstall::LuaExtractFiles(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    pInstaller->ExtractFiles();
    return 0;
}

int CBaseInstall::LuaExecuteCMD(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *cmd = luaL_checkstring(L, 1);
    
    bool required = true;
    
    if (lua_isboolean(L, 2))
        required = lua_toboolean(L, 2);
    
    const char *path = lua_tostring(L, 3);
    
    pInstaller->ExecuteCommand(cmd, required, path);
    return 0;
}

int CBaseInstall::LuaExecuteCMDAsRoot(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *cmd = luaL_checkstring(L, 1);
    
    bool required = true;
    
    if (lua_isboolean(L, 2))
        required = lua_toboolean(L, 2);
    
    const char *path = lua_tostring(L, 3);
    
    pInstaller->ExecuteCommandAsRoot(cmd, required, path);
    return 0;
}

int CBaseInstall::LuaAskRootPW(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    pInstaller->VerifyIfInstalling();
    if (!pInstaller->m_szPassword)
        pInstaller->SetUpSU("This installation requires root(administrator) privileges in order to continue\n"
                            "Please enter the password of the root user");
    return 0;
}

int CBaseInstall::LuaSetStatusMSG(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    
    pInstaller->VerifyIfInstalling();
    
    const char *msg = luaL_checkstring(L, 1);
    pInstaller->UpdateStatusText(GetTranslation(msg));
    return 0;
}

int CBaseInstall::LuaSetStepCount(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    
    pInstaller->VerifyIfInstalling();
    
    int n = luaL_checkint(L, 1);
    pInstaller->m_sInstallSteps = n /*+ !pInstaller->m_ArchList.empty()*/;
    return 0;
}

int CBaseInstall::LuaPrintInstOutput(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    
    pInstaller->VerifyIfInstalling();

    const char *msg = luaL_checkstring(L, 1);
    pInstaller->AddInstOutput(msg);
    return 0;
}



