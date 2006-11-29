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
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <poll.h>
#include <errno.h>

#include "main.h"

// -------------------------------------
// Base Installer Class
// -------------------------------------

void CBaseInstall::Init(int argc, char **argv)
{   
    m_szBinDir = dirname(argv[0]);
    
    CMain::Init(argc, argv); // Init main, will also read config files
    
    // Obtain install variabeles from lua
    m_LuaVM.GetStrVar(&m_InstallInfo.program_name, "appname");
    m_LuaVM.GetStrVar(&m_InstallInfo.intropicname, "intropic");
    
    m_LuaVM.GetStrVar(&m_InstallInfo.archive_type, "archivetype");
    if ((m_InstallInfo.archive_type != "gzip") && (m_InstallInfo.archive_type != "bzip2") && (m_InstallInfo.archive_type != "lzma"))
        throw Exceptions::CExLua("Wrong archivetype specified! Should be gzip, bzip2 or lzma.");
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
    {
        debugline("InitArchive: No such file: %s\n", archname);
        return;
    }
        
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

    while (m_CurArchIter != m_ArchList.end())
    {
        m_szCurArchFName = m_CurArchIter->first;
        
        // Set extract command
        std::string command = "cat " + std::string(m_szCurArchFName);

        if (m_InstallInfo.archive_type == "gzip")
            command += " | gzip -cd | tar xvf -";
        else if (m_InstallInfo.archive_type == "bzip2")
            command += " | bzip2 -d | tar xvf -";
        else if (m_InstallInfo.archive_type == "lzma")
        {
            command = "(" + m_szBinDir + "/../lzma-decode " + std::string(m_szCurArchFName) +
                    " arch.tar && tar xvf arch.tar && rm arch.tar)";
            debugline("Extr cmd: %s", command.c_str());
        }
        
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
            
            CPipedCMD pipe(command.c_str(), "r");
            std::string line;
            short buffer = 100;
            
            while (pipe)
            {
                InstallThink();
                
                if (pipe.HasData())
                {
                    int ch = pipe.GetCh();
                    
                    if (ch != EOF)
                    {
                        line += (char)ch;
                        buffer--;
                        if (((char)ch == '\n') || !buffer)
                        {
                            UpdateExtrStatus(line.c_str());
                            line.clear();
                            buffer = 100;
                        }
                    }
                }
            }
            
            if (!line.empty())
                UpdateExtrStatus(line.c_str());
            
            pipe.Close(); // By calling Close() explicity its able to throw exceptions
        }
        m_CurArchIter++;
    }
    
    //SetNextStep();
}

void CBaseInstall::ExecuteCommand(const char *cmd, const char *path, bool required)
{
    VerifyIfInstalling();
    
    if (m_bAlwaysRoot)
    {
        ExecuteCommandAsRoot(cmd, path, required);
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
    
    CPipedCMD pipe(command, "r");
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
            
    pipe.Close(); // By calling Close() explicity its able to throw exceptions
}

void CBaseInstall::ExecuteCommandAsRoot(const char *cmd, const char *path, bool required)
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
        m_LuaVM.LuaError("Error: function called when install is not in progress\n");
}

void CBaseInstall::UpdateStatusText(const char *msg)
{
    if (m_sCurrentStep > 0)
    {
        m_fInstallProgress += (1.0f/(float)m_sInstallSteps)*100.0f;
        SetProgress(m_fInstallProgress);
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
/*    if ((m_InstallInfo.dest_dir_type == "select") || (m_InstallInfo.dest_dir_type == "default"))
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
    }*/
    
    m_bInstalling = true;
    
    // Init all archives (if file doesn't exist nothing will be done)
    InitArchive(CreateText("%s/instarchive_all", m_szOwnDir.c_str()));
    InitArchive(CreateText("%s/instarchive_all_%s", m_szOwnDir.c_str(), m_szCPUArch.c_str()));
    InitArchive(CreateText("%s/instarchive_%s", m_szOwnDir.c_str(), m_szOS.c_str()));
    InitArchive(CreateText("%s/instarchive_%s_%s", m_szOwnDir.c_str(), m_szOS.c_str(),
                m_szCPUArch.c_str()));

    const char *destdir = GetDestDir();
    if (FileExists(destdir))
    {
        if (!ReadAccess(destdir))
        {
            throw Exceptions::CExReadExtrDir(destdir);
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

    if (m_LuaVM.InitCall("Install"))
        m_LuaVM.DoCall();
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

void CBaseInstall::InitLua()
{
    CMain::InitLua();
    
    m_LuaVM.InitClass("cfgscreen");
    m_LuaVM.RegisterClassFunc("cfgscreen", CBaseCFGScreen::LuaAddInput, "AddInput", this);
    m_LuaVM.RegisterClassFunc("cfgscreen", CBaseCFGScreen::LuaAddCheckbox, "AddCheckbox", this);
    m_LuaVM.RegisterClassFunc("cfgscreen", CBaseCFGScreen::LuaAddRadioButton, "AddRadioButton", this);
    m_LuaVM.RegisterClassFunc("cfgscreen", CBaseCFGScreen::LuaAddDirSelector, "AddDirSelector", this);
    m_LuaVM.RegisterClassFunc("cfgscreen", CBaseCFGScreen::LuaAddCFGMenu, "AddCFGMenu", this);
    
    m_LuaVM.InitClass("inputfield");
    m_LuaVM.RegisterClassFunc("inputfield", CBaseLuaInputField::LuaGet, "Get", this);
    m_LuaVM.RegisterClassFunc("inputfield", CBaseLuaInputField::LuaSetSpace, "SetSpacing", this);
    
    m_LuaVM.InitClass("checkbox");
    m_LuaVM.RegisterClassFunc("checkbox", CBaseLuaCheckbox::LuaGet, "Get", this);
    m_LuaVM.RegisterClassFunc("checkbox", CBaseLuaCheckbox::LuaSet, "Set", this);
    
    m_LuaVM.InitClass("radiobutton");
    m_LuaVM.RegisterClassFunc("radiobutton", CBaseLuaRadioButton::LuaGet, "Get", this);
    m_LuaVM.RegisterClassFunc("radiobutton", CBaseLuaRadioButton::LuaSet, "Set", this);

    m_LuaVM.InitClass("dirselector");
    m_LuaVM.RegisterClassFunc("dirselector", CBaseLuaDirSelector::LuaGet, "Get", this);
    m_LuaVM.RegisterClassFunc("dirselector", CBaseLuaDirSelector::LuaSet, "Set", this);

    m_LuaVM.InitClass("configmenu");
    m_LuaVM.RegisterClassFunc("configmenu", CBaseLuaCFGMenu::LuaGet, "Get", this);
    m_LuaVM.RegisterClassFunc("configmenu", CBaseLuaCFGMenu::LuaAddDir, "AddDir", this);
    m_LuaVM.RegisterClassFunc("configmenu", CBaseLuaCFGMenu::LuaAddString, "AddString", this);
    m_LuaVM.RegisterClassFunc("configmenu", CBaseLuaCFGMenu::LuaAddList, "AddList", this);
    m_LuaVM.RegisterClassFunc("configmenu", CBaseLuaCFGMenu::LuaAddBool, "AddBool", this);

    m_LuaVM.RegisterFunction(LuaGetTempDir, "GetTempDir", NULL, this);
    m_LuaVM.RegisterFunction(LuaNewCFGScreen, "NewCFGScreen", NULL, this);
    m_LuaVM.RegisterFunction(LuaExtractFiles, "ExtractFiles", NULL, this);
    m_LuaVM.RegisterFunction(LuaExecuteCMD, "Execute", NULL, this);
    m_LuaVM.RegisterFunction(LuaExecuteCMDAsRoot, "ExecuteAsRoot", NULL, this);
    m_LuaVM.RegisterFunction(LuaAskRootPW, "AskRootPW", NULL, this);
    m_LuaVM.RegisterFunction(LuaSetStatusMSG, "SetStatus", NULL, this);
    m_LuaVM.RegisterFunction(LuaSetStepCount, "SetStepCount", NULL, this);
    
    m_LuaVM.LoadFile("config/config.lua");
    
    if (FileExists("config/run.lua"))
        m_LuaVM.LoadFile("config/run.lua");
    
    std::string dir;
    if (!GetDestDir())
    {
        const char *env = getenv("HOME");
        if (env)
            SetDestDir(env);
        else
            SetDestDir("/");
    }

    unsigned count = m_LuaVM.OpenArray("languages");
    if (count)
    {
        std::string lang;
        for (unsigned u=1; u<=count; u++)
        {
            if (m_LuaVM.GetArrayStr(u, &lang))
                m_Languages.push_back(lang);
            else
            {
                m_LuaVM.CloseArray();
                throw Exceptions::CExLua("Non string found in languages variabele");
            }
        }
        
        m_LuaVM.CloseArray();
    }

#ifndef RELEASE
    debugline("appname: %s\n", m_InstallInfo.program_name.c_str());
    debugline("version: %s\n", m_InstallInfo.version.c_str());
    debugline("archtype: %s\n", m_InstallInfo.archive_type.c_str());
    debugline("installdir: %s\n", GetDestDir());
    debugline("languages: ");
    for (std::vector<std::string>::iterator it=m_Languages.begin(); it!=m_Languages.end(); it++)
        debugline("%s ", it->c_str());
    debugline("\n");
#endif
}

bool CBaseInstall::VerifyDestDir(void)
{
    const char *dir = GetDestDir();
    
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
        if (YesNoBox(CreateText(GetTranslation("Directory %s does not exist, do you want to create it?"), dir)))
        {
            try
            {
                MKDir(dir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH));
                return true;
            }
            catch(Exceptions::CExMKDir &e)
            {
                WarnBox("%s\n%s\n%s", GetTranslation("Could not create directory"), dir, e.what());
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

int CBaseInstall::LuaGetTempDir(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    const char *ret = CreateText("%s/tmp", pInstaller->m_szOwnDir.c_str());
    
    if (!FileExists(ret))
    {
        try
        {
            MKDir(ret, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH));
        }
        catch (Exceptions::CExMKDir &e)
        {
            // Convert to lua error(lua will catch every other exception)
            luaL_error(L, e.what());
        }
    }
    
    lua_pushstring(L, ret);
    return 1;
}

int CBaseInstall::LuaNewCFGScreen(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    const char *name = (lua_gettop(L) >= 1) ? luaL_checkstring(L, 1) : NULL;
    pInstaller->m_LuaVM.CreateClass<CBaseCFGScreen *>(pInstaller->CreateCFGScreen(GetTranslation(name)), "cfgscreen");
    return 1;
}

int CBaseInstall::LuaExtractFiles(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    pInstaller->ExtractFiles();
    return 0;
}

int CBaseInstall::LuaExecuteCMD(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    const char *cmd = luaL_checkstring(L, 1);
    const char *path = lua_tostring(L, 2);
    bool required = true;
    
    if (lua_isboolean(L, 3))
        required = lua_toboolean(L, 3);
    
    pInstaller->ExecuteCommand(cmd, path, required);
    return 0;
}

int CBaseInstall::LuaExecuteCMDAsRoot(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    const char *cmd = luaL_checkstring(L, 1);
    const char *path = lua_tostring(L, 2);
    bool required = true;
    
    if (lua_isboolean(L, 3))
        required = lua_toboolean(L, 3);
    
    pInstaller->ExecuteCommandAsRoot(cmd, path, required);
    return 0;
}

int CBaseInstall::LuaAskRootPW(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    pInstaller->VerifyIfInstalling();
    if (!pInstaller->m_szPassword)
        pInstaller->SetUpSU("This installation requires root(administrator) privileges in order to continue\n"
                            "Please enter the password of the root user");
    return 0;
}

int CBaseInstall::LuaSetStatusMSG(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    
    pInstaller->VerifyIfInstalling();
    
    const char *msg = luaL_checkstring(L, 1);
    pInstaller->UpdateStatusText(GetTranslation(msg));
    return 0;
}

int CBaseInstall::LuaSetStepCount(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    
    pInstaller->VerifyIfInstalling();
    
    int n = luaL_checkint(L, 1);
    pInstaller->m_sInstallSteps = n + !pInstaller->m_ArchList.empty();
    return 0;
}

// -------------------------------------
// Base install config screen Class
// -------------------------------------

int CBaseCFGScreen::LuaAddInput(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseCFGScreen *screen = pInstaller->m_LuaVM.CheckClass<CBaseCFGScreen *>("cfgscreen", 1);
    const char *desc = luaL_checkstring(L, 2);
    const char *label = luaL_checkstring(L, 3);
    int maxc = luaL_optint(L, 4, 1024);
    const char *val = luaL_optstring(L, 5, "");
    const char *type = luaL_optstring(L, 6, "string");
    
    if (strcmp(type, "string") && strcmp(type, "number") && strcmp(type, "float"))
        type = "string";
    
    pInstaller->m_LuaVM.CreateClass<CBaseLuaInputField *>(screen->CreateInputField(GetTranslation(desc), GetTranslation(label), val, maxc, type), "inputfield");
    
    return 1;
}

int CBaseCFGScreen::LuaAddCheckbox(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseCFGScreen *screen = pInstaller->m_LuaVM.CheckClass<CBaseCFGScreen *>("cfgscreen", 1);
    const char *desc = luaL_optstring(L, 2, "");
    
    luaL_checktype(L, 3, LUA_TTABLE);
    int count = luaL_getn(L, 3);
    std::vector<std::string> l(count);
    
    for (int i=1; i<=count; i++)
    {
        lua_rawgeti(L, 3, i);
        const char *s = luaL_checkstring(L, -1);
        l[i-1] = s;
        lua_pop(L, 1);
    }
    
    pInstaller->m_LuaVM.CreateClass<CBaseLuaCheckbox *>(screen->CreateCheckbox(GetTranslation(desc), l), "checkbox");
    
    return 1;
}

int CBaseCFGScreen::LuaAddRadioButton(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseCFGScreen *screen = pInstaller->m_LuaVM.CheckClass<CBaseCFGScreen *>("cfgscreen", 1);
    const char *desc = luaL_optstring(L, 2, "");
    
    luaL_checktype(L, 3, LUA_TTABLE);
    int count = luaL_getn(L, 3);
    std::vector<std::string> l(count);

    
    for (int i=1; i<=count; i++)
    {
        lua_rawgeti(L, 3, i);
        const char *s = luaL_checkstring(L, -1);
        l[i-1] = s;
        lua_pop(L, 1);
    }
    
    pInstaller->m_LuaVM.CreateClass<CBaseLuaRadioButton *>(screen->CreateRadioButton(GetTranslation(desc), l), "radiobutton");

    return 1;
}

int CBaseCFGScreen::LuaAddDirSelector(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseCFGScreen *screen = pInstaller->m_LuaVM.CheckClass<CBaseCFGScreen *>("cfgscreen", 1);
    const char *desc = luaL_checkstring(L, 2);
    const char *val = luaL_optstring(L, 3, "");
    
    pInstaller->m_LuaVM.CreateClass<CBaseLuaDirSelector *>(screen->CreateDirSelector(GetTranslation(desc), val), "dirselector");
    
    return 1;
}

int CBaseCFGScreen::LuaAddCFGMenu(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseCFGScreen *screen = pInstaller->m_LuaVM.CheckClass<CBaseCFGScreen *>("cfgscreen", 1);
    const char *desc = luaL_checkstring(L, 2);
    
    pInstaller->m_LuaVM.CreateClass<CBaseLuaCFGMenu *>(screen->CreateCFGMenu(GetTranslation(desc)), "configmenu");
    
    return 1;
}

// -------------------------------------
// Base Lua Inputfield Class
// -------------------------------------

CBaseLuaInputField::CBaseLuaInputField(const char *t)
{
    if (!t || !t[0] || (strcmp(t, "number") && strcmp(t, "float") && strcmp(t, "string")))
        m_szType = "string";
    else
        m_szType = t;
}

int CBaseLuaInputField::LuaGet(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaInputField *field = pInstaller->m_LuaVM.CheckClass<CBaseLuaInputField *>("inputfield", 1);
    
    if (field->GetType() == "string")
        lua_pushstring(L, field->GetValue());
    else if (field->GetType() == "number")
        lua_pushinteger(L, atoi(field->GetValue()));
    else if (field->GetType() == "float")
        lua_pushnumber(L, atof(field->GetValue()));
    
    return 1;
}

int CBaseLuaInputField::LuaSetSpace(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaInputField *field = pInstaller->m_LuaVM.CheckClass<CBaseLuaInputField *>("inputfield", 1);
    int percent = luaL_checkint(L, 2);
    field->SetSpacing(percent);
    return 0;
}

// -------------------------------------
// Base Lua Checkbox Class
// -------------------------------------

int CBaseLuaCheckbox::LuaGet(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaCheckbox *box = pInstaller->m_LuaVM.CheckClass<CBaseLuaCheckbox *>("checkbox", 1);
    int n = luaL_checkint(L, 2);
    lua_pushboolean(L, box->Enabled(n));
    return 1;
}

int CBaseLuaCheckbox::LuaSet(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaCheckbox *box = pInstaller->m_LuaVM.CheckClass<CBaseLuaCheckbox *>("checkbox", 1);
    int n = luaL_checkint(L, 2);
    box->Enable(n);
    return 0;
}

// -------------------------------------
// Base Lua Radio Button Class
// -------------------------------------

int CBaseLuaRadioButton::LuaGet(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaRadioButton *box = pInstaller->m_LuaVM.CheckClass<CBaseLuaRadioButton *>("radiobutton", 1);
    lua_pushinteger(L, box->EnabledButton());
    return 1;
}

int CBaseLuaRadioButton::LuaSet(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaRadioButton *box = pInstaller->m_LuaVM.CheckClass<CBaseLuaRadioButton *>("radiobutton", 1);
    int n = luaL_checkint(L, 2);
    box->Enable(n);
    return 0;
}

// -------------------------------------
// Base Lua Directory Selector Class
// -------------------------------------

int CBaseLuaDirSelector::LuaGet(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaDirSelector *sel = pInstaller->m_LuaVM.CheckClass<CBaseLuaDirSelector *>("dirselector", 1);
    lua_pushstring(L, sel->GetDir());
    return 1;
}

int CBaseLuaDirSelector::LuaSet(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaDirSelector *sel = pInstaller->m_LuaVM.CheckClass<CBaseLuaDirSelector *>("dirselector", 1);
    const char *dir = luaL_checkstring(L, 2);
    sel->SetDir(dir);
    return 0;
}

// -------------------------------------
// Base Lua config menu Class
// -------------------------------------

CBaseLuaCFGMenu::~CBaseLuaCFGMenu()
{
    for (std::map<std::string, entry_s *>::iterator it=m_Variabeles.begin(); it!=m_Variabeles.end(); it++)
        delete it->second;
}

void CBaseLuaCFGMenu::AddVar(const char *name, const char *desc, const char *val, EVarType type, TOptionsType *l)
{
    if (m_Variabeles[name])
    {
        delete m_Variabeles[name];
        m_Variabeles[name] = NULL;
    }
    
    const char *value = val;
    
    if (l)
        std::sort(l->begin(), l->end());
    
    if (!value || !*value)
    {
        switch (type)
        {
            case TYPE_DIR:
                value = "/";
                break;
            case TYPE_STRING:
                value = "";
                break;
            case TYPE_LIST:
                if (l)
                    value = l->front().c_str();
                else
                    value = "";
                break;
            case TYPE_BOOL:
                value = "Disable";
                break;
        }
    }
    
    m_Variabeles[name] = new entry_s(value, desc, type);
    
    if (l)
    {
        m_Variabeles[name]->options = *l;
    }
}

int CBaseLuaCFGMenu::LuaAddDir(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaCFGMenu *menu = pInstaller->m_LuaVM.CheckClass<CBaseLuaCFGMenu *>("configmenu", 1);
    const char *var = luaL_checkstring(L, 2);
    const char *desc = luaL_checkstring(L, 3);
    const char *val = luaL_optstring(L, 4, "/");

    menu->AddVar(var, GetTranslation(desc), val, TYPE_DIR);
    
    return 0;
}

int CBaseLuaCFGMenu::LuaAddString(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaCFGMenu *menu = pInstaller->m_LuaVM.CheckClass<CBaseLuaCFGMenu *>("configmenu", 1);
    const char *var = luaL_checkstring(L, 2);
    const char *desc = luaL_checkstring(L, 3);
    const char *val = lua_tostring(L, 4);

    menu->AddVar(var, GetTranslation(desc), val, TYPE_STRING);
    
    return 0;
}

int CBaseLuaCFGMenu::LuaAddList(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaCFGMenu *menu = pInstaller->m_LuaVM.CheckClass<CBaseLuaCFGMenu *>("configmenu", 1);
    const char *var = luaL_checkstring(L, 2);
    const char *desc = luaL_checkstring(L, 3);
    
    luaL_checktype(L, 4, LUA_TTABLE);
    int count = luaL_getn(L, 4);
    TOptionsType l;
    
    for (int i=1; i<=count; i++)
    {
        lua_rawgeti(L, 4, i);
        const char *s = luaL_checkstring(L, -1);
        l.push_back(s);
        lua_pop(L, 1);
    }
    
    if (l.empty())
        luaL_error(L, "Empty list given");

    const char *val = lua_tostring(L, 5);
    
    menu->AddVar(var, GetTranslation(desc), val, TYPE_LIST, &l);
    
    return 0;
}

int CBaseLuaCFGMenu::LuaAddBool(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaCFGMenu *menu = pInstaller->m_LuaVM.CheckClass<CBaseLuaCFGMenu *>("configmenu", 1);
    const char *var = luaL_checkstring(L, 2);
    const char *desc = luaL_checkstring(L, 3);
    bool val = (lua_isboolean(L, 4)) ? lua_toboolean(L, 4) : false;

    TOptionsType l;
    l.push_back("Disable");
    l.push_back("Enable");
    
    menu->AddVar(var, GetTranslation(desc), menu->BoolToStr(val), TYPE_BOOL, &l);
    
    return 0;
}

int CBaseLuaCFGMenu::LuaGet(lua_State *L)
{
    CBaseInstall *pInstaller = (CBaseInstall *)lua_touserdata(L, lua_upvalueindex(1));
    CBaseLuaCFGMenu *menu = pInstaller->m_LuaVM.CheckClass<CBaseLuaCFGMenu *>("configmenu", 1);
    const char *var = luaL_checkstring(L, 2);
    
    if (menu->m_Variabeles[var])
    {
        if (menu->m_Variabeles[var]->type == TYPE_BOOL)
            lua_pushboolean(L, (menu->m_Variabeles[var]->val == "Enable") ? true : false);
        else
            lua_pushstring(L, menu->m_Variabeles[var]->val.c_str());
    }
    else
        luaL_error(L, "No variabele %s found in menu\n", var);
    
    return 1;
}

