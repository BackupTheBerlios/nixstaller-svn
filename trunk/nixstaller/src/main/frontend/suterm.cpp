/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "suterm.h"

namespace {

const char *TermStr = "Im Done Now :)";

}

// -------------------------------------
// Su terminal Class
// -------------------------------------

CSuTerm::ESuType CSuTerm::m_eSuType = TYPE_UNKNOWN;
std::string CSuTerm::m_RunnerPath;

CSuTerm::CSuTerm(const std::string &user) : m_User(user)
{
    if (m_RunnerPath.empty())
        throw Exceptions::CExSuError("No su runner path defined");
}

void CSuTerm::SetDefSuType()
{
    if (m_eSuType == TYPE_MAYBESUDO)
        m_eSuType = TYPE_SUDO;
    else if (m_eSuType == TYPE_MAYBESU)
        m_eSuType = TYPE_SU;
}

const char *CSuTerm::GetSuBin(ESuType sutype) const
{
    if ((sutype == TYPE_MAYBESUDO) || (sutype == TYPE_SUDO))
    {
        if (FileExists("/usr/bin/sudo"))
            return "/usr/bin/sudo";
        else if (FileExists("/bin/sudo"))
            return "/bin/sudo";
    }
    else if ((sutype == TYPE_MAYBESU) || (sutype == TYPE_SU))
    {
        if (FileExists("/usr/bin/su"))
            return "/usr/bin/su";
        else if (FileExists("/bin/su"))
            return "/bin/su";
    }
    
    return NULL;
}

void CSuTerm::PrepareExec()
{
    if (UsingSudo())
    {
         // Forget about any previous sessions. This is so that
         // NeedPassword() TestPassword() cannot wrongly report that no pw is
         // needed when Exec() is called outside sudo's timeout.
        Exec("sudo -k");
        CheckPidExited(true);
    }
}

std::string CSuTerm::ConstructCommand(std::string command)
{
    if (m_eSuType == TYPE_UNKNOWN)
    {
        // Always try sudo first (when unconfigured it may work
        // with the pw of target user)
        if (GetSuBin(TYPE_SUDO))
            m_eSuType = TYPE_MAYBESUDO;
        else
            m_eSuType = TYPE_MAYBESU;
    }
    
    std::string ret = GetSuBin(m_eSuType);
    
    if (UsingSudo())
        ret += " -u";
    
    ret += " " + m_User;
    
    if (UsingSudo())
        ret += " -i -- -c";
    else
        ret += " -c";

//     command = CreateText("%s/surunner %d \'%s\'", m_RunnerPath.c_str(), getpid(), command.c_str());
    command = "/bin/sh -c \'" + command + "\'";

    // 'Escape' single quotes by surrounding them with double quotes
    // UNDONE: Move replace code to generic function?
    const char *quotehack = "\'\"\'\"\'"; // String to replace single quotes
    const TSTLStrSize qlen = strlen(quotehack);
    TSTLStrSize start = 0;
    while (start < command.length()) // NOTE: length chances so it's checked each time
    {
        start = command.find("\'", start);
        if (start != std::string::npos)
        {
            command.replace(start, 1, quotehack);
            start += qlen;
            continue;
        }
        else
            break;
        
        start++;
    }
    
    // Insert our magic terminate indicator and command
    ret += std::string(" 'printf \"") + TermStr + "\\n\" ; " + command + "'";
    
    if (!UsingSudo())
        ret += " -";

    debugline("ConstructCommand: %s\n", ret.c_str());
    return ret;
}

bool CSuTerm::WaitSlave()
{
    CFDWrapper slave = open(GetTTYName().c_str(), O_RDWR);
    if (slave < 0)
        return false;

    struct termios tio;
    while (IsValid())
    {
        if (!CheckPid())
            return false;
        
        if (tcgetattr(slave, &tio) < 0)
            return false;
        
        if (tio.c_lflag & ECHO) 
        {
            CheckForData();
            continue;
        }
        break;
    }
    
    return true;
}

CSuTerm::ESuTalkStat CSuTerm::TalkWithSu(const char *password)
{
    enum { WaitForPrompt, CheckStar, WaitTillDone } state = WaitForPrompt;
    std::string line;
    
    while (IsValid())
    {
        if (!CheckForData())
            continue;
        
        ReadLine(line, false);
//         debugline("ReadLine: %s\n", line.c_str());

        switch (state)
        {
            case WaitForPrompt:
            {
                if (line.empty())
                    return SU_AUTHERROR;
                
                // In case no password is needed.
                if (!line.compare(0, strlen(TermStr), TermStr))
                {
                    debugline("we can skip pw :)\n");
                    return SU_OK;
                }

                while (IsValid())
                {
                    // There is more output available, so the previous line
                    // couldn't have been a password prompt (the definition
                    // of prompt being that  there's a line of output followed 
                    // by a colon, and then the process waits).
                    std::string more;
                    ReadLine(more);
                    if (more.empty())
                        break;
                    line = more;
                }
           
                // Match "Password: " with the regex ^[^:]+:[\w]*$.
                TSTLStrSize i, j, colon, length = line.length();
                for (i=0, j=0, colon=0; i<length; i++)
                {
                    if (line[i] == ':')
                    {
                        j = i;
                        colon++;
                        continue;
                    }
                    
                    if (!isspace(line[i]))
                        j++;
                }
                
                if ((colon == 1) && (line[j] == ':'))
                {
                    if (!password)
                        return SU_NULLPASS;
                    
                    if (!CheckPid())
                        return SU_ERROR;
                    
                    if (WaitSlave())
                    {
                        write(GetPTYFD(), password, strlen(password));
                        write(GetPTYFD(), "\n", 1);
                        debugline("Wrote pw\n");
                        state = CheckStar;
                    }
                    else
                        return SU_ERROR;
                }
                break;
            }
            
            case CheckStar:
            {
                TSTLStrSize pos1 = line.find_first_not_of(" \r\t\n");
                TSTLStrSize pos2 = line.find_last_not_of(" \r\t\n");
                std::string s;
                 
                if (pos1 != std::string::npos)
                    s = line.substr(pos1, (pos2==std::string::npos) ? pos2 : (pos2-pos1));
                
                if (s.empty())
                {
                    state = WaitTillDone;
                    break;
                }
                
                TSTLStrSize n, length = s.length();
                for (n=0; n<length; n++)
                {
                    if (s[n] != '*')
                        return SU_ERROR;
                }
                
                state = WaitTillDone;
                break;
            }
            
            case WaitTillDone:
                if (line.empty())
                    return SU_AUTHERROR;

                // Read till we get our personal terminate string...
                if (!line.compare(0, strlen(TermStr), TermStr))
                    return SU_OK;
                else if (UsingSudo())
                    return SU_AUTHERROR;
                break;
        }
    }
    
    return SU_ERROR;
}

bool CSuTerm::NeedPassword()
{
    PrepareExec();
    Exec(ConstructCommand(""));
    ESuTalkStat ret = TalkWithSu(NULL);

    if (ret != SU_OK)
        Abort();

    if (ret == SU_ERROR)
        throw Exceptions::CExSuError("su/sudo IO error");
    else if (ret == SU_NULLPASS)
    {
        Abort();
        return true;
    }
    else if (ret == SU_AUTHERROR)
    {
        Abort();

        if (m_eSuType == TYPE_MAYBESUDO)
        {
            m_eSuType = TYPE_MAYBESU; // Try again with su
            return NeedPassword();
        }
        else
            m_eSuType = TYPE_UNKNOWN;
        
        return false;
    }

    SetDefSuType();
    
    CheckPidExited(true);
    
    return false;
}

bool CSuTerm::TestPassword(const char *password)
{
    PrepareExec();
    Exec(ConstructCommand(""));
    ESuTalkStat ret = TalkWithSu(password);

    if (ret != SU_OK)
        Abort();
    
    if (ret == SU_ERROR)
        throw Exceptions::CExSuError("su/sudo IO error");
    else if (ret == SU_NULLPASS)
        return false;
    else if (ret == SU_AUTHERROR)
    {
        Abort();
        
        if (m_eSuType == TYPE_MAYBESUDO)
        {
            m_eSuType = TYPE_MAYBESU; // Try again with su

            if (TestPassword(password))
            {
                m_eSuType = TYPE_SU;
                return true;
            }
            else
                m_eSuType = TYPE_UNKNOWN;
        }
        
        return false;
    }

    SetDefSuType();
    
    CheckPidExited(true);
    
    return true;
}

void CSuTerm::Exec(const std::string &command, const char *password)
{
    Exec(ConstructCommand(command));
    ESuTalkStat ret = TalkWithSu(password);

    debugline("Exec: %d\n", ret);

    if (ret != SU_OK)
        Abort();
    
    if (ret == SU_ERROR)
        throw Exceptions::CExSuError("su/sudo IO error");
    else if (ret == SU_NULLPASS)
        // Exception because NeedPassword() should be used
        throw Exceptions::CExSuError("No password given");
    else if (ret == SU_AUTHERROR)
    {
        if (m_eSuType == TYPE_MAYBESUDO)
        {
            m_eSuType = TYPE_MAYBESU; // Try again with su
            Exec(command, password);
            m_eSuType = TYPE_SU; // If we are here no exception was thrown and it worked...
        }
        else
        {
            m_eSuType = TYPE_UNKNOWN;
            throw Exceptions::CExSuError("Failed to authenticate with su and/or sudo");
        }
    }

    SetDefSuType();
}
