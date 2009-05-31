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

std::string CSuTerm::ConstructCommand(const std::string &origcmd)
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

    // Insert our magic terminate indicator to command
    ret += std::string(" printf \"") + TermStr + "\\n\" ;" + origcmd;
    
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
            HasData(); // UNDONE? (Cleaner way to wait for data)
            continue;
        }
        break;
    }
    
    return true;
}

CSuTerm::ESuTalkStat CSuTerm::TalkWithSU(const char *password)
{
    enum { WaitForPrompt, CheckStar, WaitTillDone } state = WaitForPrompt;
    std::string line;
    
    while (IsValid())
    {
        if (!HasData())
            continue;
        
        EReadStatus readstat = ReadLine(line);

        if (readstat == READ_AGAIN)
            continue;
        
        debugline("TalkWithSU: %s\n", line.c_str());

        switch (state)
        {
            case WaitForPrompt:
            {
                if (line.empty())
                    return SU_AUTHERROR;
                
                // In case no password is needed.
                if (!line.compare(0, strlen(TermStr), TermStr))
                    return SU_OK;

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
                        j = i; colon++;
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
                        state = CheckStar;
                    }
                    else
                        return SU_ERROR;
                }
                break;
            }
            
            case CheckStar:
            {
                std::string::size_type pos1 = line.find_first_not_of(" \r\t\n");
                std::string::size_type pos2 = line.find_last_not_of(" \r\t\n");
                
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
    
    return SU_OK;
}

bool CSuTerm::NeedPassword()
{
    Exec(ConstructCommand(""));
    ESuTalkStat ret = TalkWithSU(NULL);
    
    if (ret == SU_NULLPASS)
    {
        Abort();
        return true;
    }
    else if (ret == SU_AUTHERROR)
    {
        Abort();

        if (m_eSuType == TYPE_MAYBESUDO)
            m_eSuType = TYPE_MAYBESU; // Try again with su
        else
            m_eSuType = TYPE_UNKNOWN;
        
        return false;
    }

    SetDefSuType();
    
    CheckPidExited(true);
    
    return false;
}
