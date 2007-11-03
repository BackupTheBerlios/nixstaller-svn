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
#include "main/install/luainput.h"
#include "main/install/lualabel.h"
#include "main/install/luamenu.h"
#include "main/install/luaprogressbar.h"
#include "main/install/luaradiobutton.h"
#include "main/install/luatextfield.h"
#include "main/lua/lua.h"
#include "main/lua/luaclass.h"
#include "main/lua/luafunc.h"
#include "main/lua/luatable.h"

// -------------------------------------
// Base Installer Class
// -------------------------------------

CBaseInstall::CBaseInstall(void) : m_ulTotalArchSize(1), m_fExtrPercent(0.0f), m_szCurArchFName(NULL),
                                   m_bAlwaysRoot(false), m_sInstallSteps(1), m_sCurrentStep(0),
                                   m_fInstallProgress(0.0f), m_iUpdateStatLuaFunc(LUA_NOREF),
                                   m_iUpdateProgLuaFunc(LUA_NOREF), m_iUpdateOutputLuaFunc(LUA_NOREF),
                                   m_bInstalling(false)
{
}

void CBaseInstall::AddScreen(CBaseScreen *screen)
{
    m_ScreenList.push_back(screen);
    CoreAddScreen(screen);
}

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
    
    lua_pushlightuserdata(NLua::LuaState, this);
    lua_setfield(NLua::LuaState, LUA_REGISTRYINDEX, "installer");
    NLua::LuaSetHook(LuaHook);
}

void CBaseInstall::CoreUpdateLanguage()
{
    for (TScreenList::iterator it=m_ScreenList.begin(); it!=m_ScreenList.end(); it++)
        (*it)->UpdateLanguage();
}

void CBaseInstall::Install(int statluafunc, int progluafunc, int outluafunc)
{
    if (!ChoiceBox(CreateText(GetTranslation("This will install %s\nContinue?"), m_InstallInfo.program_name.c_str()),
         GetTranslation("Exit program"), GetTranslation("Continue"), NULL))
        throw Exceptions::CExUser();
    
    m_bInstalling = true;
    m_iUpdateStatLuaFunc = statluafunc;
    m_iUpdateProgLuaFunc = progluafunc;
    m_iUpdateOutputLuaFunc = outluafunc;
    
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
            // Dirselector screens should also check for RO dirs,
            // so this basicly only happens when the installer defaults to a single dir
        }
        else if (!WriteAccess(destdir))
        {
            m_bAlwaysRoot = true;
            GetSUPasswd("This installation requires root(administrator) privileges in order to continue\n"
                        "Please enter the password of the root user", true);
        }
    }
    else
    {
        AddOutput("Creating destination directory...");
        
        if (MKDirNeedsRoot(destdir))
        {
            m_bAlwaysRoot = true;
            GetSUPasswd("This installation requires root(administrator) privileges in order to continue\n"
                        "Please enter the password of the root user", true);

            MKDirRecRoot(destdir, m_SUHandler, m_szPassword);
        }
        else
            MKDirRec(destdir);
    }
    
    CHDir(destdir);
    
    m_SUHandler.SetThinkFunc(SUThinkFunc, this);

    NLua::CLuaFunc func("Install");
    
    if (func)
        func(0);
    else
        ExtractFiles(); // Default behaviour
    
//     AddOutput("Registering installation...");
//     RegisterInstall();
//     CalcSums();
//     //Register.CheckSums(m_InstallInfo.program_name.c_str());
//     AddOutput("done\n");

    SetProgress(100);
    MsgBox(GetTranslation("Installation of %s complete!"), m_InstallInfo.program_name.c_str());
    CleanPasswdString(m_szPassword);
    m_szPassword = NULL;
    m_bInstalling = false;
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
            std::string tarname = m_szOwnDir + "/arch.tar";
            command = "(" + m_szBinDir + "/../lzma-decode " + std::string(m_szCurArchFName) +
                    " " + tarname + " >/dev/null 2>&1 && tar xvf " + tarname + ")";
        }
        
        debugline("Extr cmd: %s", command.c_str());
        
        if (m_bAlwaysRoot)
        {
            m_SUHandler.SetCommand(command);
            if (!m_SUHandler.ExecuteCommand(m_szPassword))
            {
                debugline("Error extracting files\n");
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
                UpdateUI();
                
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

int CBaseInstall::ExecuteCommand(const char *cmd, bool required, const char *path)
{
    VerifyIfInstalling();
    
    if (m_bAlwaysRoot)
        return ExecuteCommandAsRoot(cmd, required, path);
    
    // Redirect stderr to stdout, so that errors will be displayed too
    const char *append = " 2>&1";
    char command[strlen(cmd) + strlen(append)];
    strcpy(command, cmd);
    strcat(command, append);
    
    if (path && *path)
        setenv("PATH", path, 1);
    else
        setenv("PATH", GetDefaultPath(), 1);
    
    AddOutput(CreateText("\nExecute: %s\n\n", cmd));
    
    CPipedCMD pipe(command);
    std::string line;
            
    while (pipe)
    {
        UpdateUI();
                
        if (pipe.HasData())
        {
            int ch = pipe.GetCh();
                    
            if (ch != EOF)
            {
                line += (char)ch;
                if ((char)ch == '\n')
                {
                    AddOutput(line.c_str());
                    line.clear();
                }
            }
        }
    }
            
    if (!line.empty())
        AddOutput(line.c_str());
            
    return pipe.Close(required); // By calling Close() explicity its able to throw exceptions
}

int CBaseInstall::ExecuteCommandAsRoot(const char *cmd, bool required, const char *path)
{
    VerifyIfInstalling();
    
    GetSUPasswd("This installation requires root(administrator) privileges in order to continue\n"
            "Please enter the password of the root user", true);
    
    m_SUHandler.SetOutputFunc(CMDSUOutFunc, this);

    if (!path || !path[0])
        path = GetDefaultPath();
    
    m_SUHandler.SetPath(path);
    m_SUHandler.SetCommand(cmd);
    
    AddOutput(CreateText("\nExecute: %s\n\n", cmd));
    
    if (!m_SUHandler.ExecuteCommand(m_szPassword))
    {
        if (required)
        {
            CleanPasswdString(m_szPassword);
            m_szPassword = NULL;
            throw Exceptions::CExCommand(cmd);
        }
    }
    return m_SUHandler.Ret();
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
    
    if (m_iUpdateStatLuaFunc != LUA_NOREF)
    {
        NLua::CLuaFunc func(m_iUpdateStatLuaFunc, LUA_REGISTRYINDEX);
        
        if (func)
        {
            if (m_sInstallSteps > 1)
                func << CreateText("%s: %s (%d/%d)", GetTranslation("Status"),
                                   GetTranslation(msg), m_sCurrentStep, m_sInstallSteps);
            else
                func << CreateText("%s: %s", GetTranslation("Status"), GetTranslation(msg));
            
            func(0);
        }
    }
}

void CBaseInstall::AddOutput(const std::string &str)
{
    if (m_iUpdateOutputLuaFunc != LUA_NOREF)
    {
        NLua::CLuaFunc func(m_iUpdateOutputLuaFunc, LUA_REGISTRYINDEX);
        
        if (func)
        {
            func << str;
            func(0);
        }
    }
}

void CBaseInstall::SetProgress(int percent)
{
    if (m_iUpdateProgLuaFunc != LUA_NOREF)
    {
        NLua::CLuaFunc func(m_iUpdateProgLuaFunc, LUA_REGISTRYINDEX);
        
        if (func)
        {
            func << percent;
            func(0);
        }
    }
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
    
    m_fExtrPercent += (((float)m_ArchList[m_szCurArchFName].filesizes[stat]/(float)m_ulTotalArchSize)*100.0f);
    if (m_fExtrPercent < 0.0f)
    {
        m_fExtrPercent = 0.0f;
        debugline("m_fExtrPercent below 0!\n");
    }
    else if (m_fExtrPercent > 100.0f)
    {
        debugline("m_fExtrPercent above max: %f!\n", m_fExtrPercent);
        m_fExtrPercent = 100.0f;
    }

    AddOutput("Extracting file: " + stat + '\n');
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
    CBaseLuaLabel::LuaRegister();

    NLua::RegisterFunction(LuaNewScreen, "newscreen", "install", this);
    NLua::RegisterFunction(LuaAddScreen, "addscreen", "install", this);
    NLua::RegisterFunction(LuaGetTempDir, "gettempdir", "install", this);
    NLua::RegisterFunction(LuaGetPkgDir, "getpkgdir", "install", this);
    NLua::RegisterFunction(LuaExtractFiles, "extractfiles", "install", this);
    NLua::RegisterFunction(LuaExecuteCMD, "execute", "install", this);
    NLua::RegisterFunction(LuaExecuteCMDAsRoot, "executeasroot", "install", this);
    NLua::RegisterFunction(LuaAskRootPW, "askrootpw", "install", this);
    NLua::RegisterFunction(LuaSetStatusMSG, "setstatus", "install", this);
    NLua::RegisterFunction(LuaSetStepCount, "setstepcount", "install", this);
    NLua::RegisterFunction(LuaPrintInstOutput, "print", "install", this);
    NLua::RegisterFunction(LuaStartInstall, "startinstall", "install", this);
    NLua::RegisterFunction(LuaLockScreen, "lockscreen", "install", this);
    NLua::RegisterFunction(LuaVerifyDestDir, "verifydestdir", "install", this);
    NLua::RegisterFunction(LuaExtraFilesPath, "extrafilespath", "install", this);
    NLua::RegisterFunction(LuaGetLang, "getlang", "install", this);
    NLua::RegisterFunction(LuaSetLang, "setlang", "install", this);
    
    const char *env = getenv("HOME");
    if (env)
        SetDestDir(env);
    else
        SetDestDir("/");
    
    NLua::LoadFile("config/config.lua");
    NLua::LoadFile("config/package.lua");
    NLua::LoadFile("install.lua");
    
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

void CBaseInstall::DeleteScreens()
{
    debugline("Deleting %u screens...\n", m_ScreenList.size());
    while (!m_ScreenList.empty())
    {
        delete m_ScreenList.back();
        m_ScreenList.pop_back();
    }
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
                if (MKDirNeedsRoot(dir))
                {
                    GetSUPasswd("Your account doesn't have permissions to "
                            "create the directory.\nTo create it with the root "
                            "(administrator) account, please enter it's password below.", false);
                    MKDirRecRoot(dir, m_SUHandler, m_szPassword);
                }
                else
                    MKDirRec(dir);
            }
            catch(Exceptions::CExIO &e)
            {
                WarnBox(e.what());
                return false;
            }
            
            return true;
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

void CBaseInstall::LuaHook(lua_State *L, lua_Debug *ar)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "installer");
    CBaseInstall *pInstaller = static_cast<CBaseInstall *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
        
    try
    {
        pInstaller->UpdateUI();
    }
    catch(Exceptions::CException &e)
    {
        ConvertExToLuaError();
    }
}

int CBaseInstall::LuaNewScreen(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *name = luaL_optstring(L, 1, "");
    NLua::CreateClass(pInstaller->CreateScreen(name), "screen");
    return 1;
}

int CBaseInstall::LuaAddScreen(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    CBaseScreen *screen = NLua::CheckClassData<CBaseScreen>("screen", 1);
    pInstaller->AddScreen(screen);
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

int CBaseInstall::LuaGetPkgDir(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *ret = CreateText("%s/pkg/files", pInstaller->m_szOwnDir.c_str());
    
    if (!FileExists(ret))
        MKDirRec(ret);
    
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
    
    lua_pushinteger(L, pInstaller->ExecuteCommand(cmd, required, path));
    return 1;
}

int CBaseInstall::LuaExecuteCMDAsRoot(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *cmd = luaL_checkstring(L, 1);
    
    bool required = true;
    
    if (lua_isboolean(L, 2))
        required = lua_toboolean(L, 2);
    
    const char *path = lua_tostring(L, 3);
    
    lua_pushinteger(L, pInstaller->ExecuteCommandAsRoot(cmd, required, path));
    return 1;
}

int CBaseInstall::LuaAskRootPW(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    pInstaller->VerifyIfInstalling();
    pInstaller->GetSUPasswd("This installation requires root(administrator) privileges in order to continue\n"
                        "Please enter the password of the root user", true);
    return 0;
}

int CBaseInstall::LuaSetStatusMSG(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    
    pInstaller->VerifyIfInstalling();
    
    const char *msg = luaL_checkstring(L, 1);
    pInstaller->UpdateStatusText(msg);
    return 0;
}

int CBaseInstall::LuaSetStepCount(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    
    pInstaller->VerifyIfInstalling();
    
    int n = luaL_checkint(L, 1);
    pInstaller->m_sInstallSteps = n;
    return 0;
}

int CBaseInstall::LuaPrintInstOutput(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    
    pInstaller->VerifyIfInstalling();

    const char *msg = luaL_checkstring(L, 1);
    pInstaller->AddOutput(msg);
    return 0;
}

int CBaseInstall::LuaStartInstall(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    
    if (pInstaller->m_bInstalling)
        return 0;
    
    int stat = LUA_NOREF, prog = LUA_NOREF, out = LUA_NOREF;
    
    if (lua_isfunction(NLua::LuaState, 1))
        stat = NLua::MakeReference(1);
    
    if (lua_isfunction(NLua::LuaState, 2))
        prog = NLua::MakeReference(2);

    if (lua_isfunction(NLua::LuaState, 3))
        out = NLua::MakeReference(3);

    pInstaller->Install(stat, prog, out);
    
    NLua::Unreference(stat);
    NLua::Unreference(prog);
    NLua::Unreference(out);
    
    return 0;
}

int CBaseInstall::LuaLockScreen(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    pInstaller->LockScreen(NLua::LuaToBool(1), NLua::LuaToBool(2), NLua::LuaToBool(3));
    return 0;
}

int CBaseInstall::LuaVerifyDestDir(lua_State *L)
{
    lua_pushboolean(L, GetFromClosure(L)->VerifyDestDir());
    return 1;
}

int CBaseInstall::LuaExtraFilesPath(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    const char *file = luaL_optstring(L, 1, "");
    lua_pushfstring(L, "%s/files_extra/%s", pInstaller->m_szOwnDir.c_str(), file);
    return 1;
}

int CBaseInstall::LuaGetLang(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    lua_pushstring(L, pInstaller->m_szCurLang.c_str());
    return 1;
}

int CBaseInstall::LuaSetLang(lua_State *L)
{
    CBaseInstall *pInstaller = GetFromClosure(L);
    pInstaller->m_szCurLang = luaL_checkstring(L, 1);
    pInstaller->UpdateLanguage();
    return 0;
}

// -------------------------------------
// Util functions
// -------------------------------------

CBaseInstall *GetFromClosure(lua_State *L)
{
    return reinterpret_cast<CBaseInstall *>(lua_touserdata(L, lua_upvalueindex(1)));
}
