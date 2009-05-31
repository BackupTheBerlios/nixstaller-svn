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

#include "suterm.h"

namespace {

const char *TermStr = "Im Done Now :)";

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

void CSuTerm::ConstructCommand(std::string &cmd, TStringVec &args,
                               const std::string &runcmd, const TStringVec &runargs)
{
    if (m_SuType == TYPE_UNKNOWN)
    {
        // Always try sudo first (when unconfigured it may work
        // with the pw of target user)
        if (GetSuBin(TYPE_SUDO))
            m_SuType = TYPE_MAYBESUDO;
        else
            m_SuType = TYPE_MAYBESU;
    }
    
    cmd = GetSuBin(m_SuType);
    
    if (UsingSudo())
        args.push_back("-u");
    
    args.push_back(m_User);
    
    if (UsingSudo())
    {
        args.push_back("-i");
        args.push_back("--");
        args.push_back("-c");
    }
    else
        args.push_back("-c");

    // Insert our magic terminate indicator to command
    args.push_back(std::string("printf \"") + TermStr + "\\n\" ;" + runcmd);
    
    if (!UsingSudo())
        args.push_back("-");
    
    args.insert(args.end(), runargs.begin(), runargs.end());
}

int CSuTerm::TalkWithSU(const char *password)
{
#if 0
    enum { WaitForPrompt, CheckStar, WaitTillDone } state = WaitForPrompt;
    int colon;
    std::string::size_type i, j;

    std::string line;
    while (true)
    {
        EReadStatus readstat = ReadLine(line);

        if (readstat == READ_AGAIN)
            continue;

        switch (state)
        {
            case WaitForPrompt:
                if (line.empty())
                {
                    SetError(SU_ERROR_INCORRECTUSER, "Incorrect user");
                    return SUCOM_NOTAUTHORIZED;
                }
                
                // In case no password is needed.
                if (!line.compare(0, strlen(TermStr), TermStr))
                {
                    //UnReadLine(line);
                    return SUCOM_OK;
                }

                while(WaitMS(m_iPTYFD, 100)>0)
                {
                    // There is more output available, so the previous line
                    // couldn't have been a password prompt (the definition
                    // of prompt being that  there's a line of output followed 
                    // by a colon, and then the process waits).
                    std::string more = ReadLine();
                    if (more.empty()) break;
                    line = more;
                    log("Read Line more: %s\n", more.c_str());
                }
           
                // Match "Password: " with the regex ^[^:]+:[\w]*$.
                for (i=0,j=0,colon=0; i<line.length(); i++)
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
                    if (password == 0L)
                    {
                        SetError(SU_ERROR_INCORRECTPASS, "Incorrect password");
                        return SUCOM_NULLPASS;
                    }
                    if (!CheckPid(m_iPid))
                    {
                        SetError(SU_ERROR_INTERNAL, "su has exited while waiting for password");
                        return SUCOM_ERROR;
                    }
                    if (WaitSlave() == 0)
                    {
                        log("Writing passwd...\n");
                        write(m_iPTYFD, password, strlen(password));
                        write(m_iPTYFD, "\n", 1);
                        state=CheckStar;
                    }
                    else
                    {
                        return SUCOM_ERROR;
                    }
                }
                break;

            case CheckStar:
            {
                std::string::size_type pos1 = line.find_first_not_of(" \r\t\n");
                std::string::size_type pos2 = line.find_last_not_of(" \r\t\n");
                
                std::string s;
                 
                if (pos1 != std::string::npos)
                    s = line.substr(pos1, (pos2==std::string::npos) ? pos2 : (pos2-pos1));
                
                if (s.empty())
                {
                    state=WaitTillDone;
                    break;
                }
                for (i=0; i<s.length(); i++)
                {
                    if (s[i] != '*')
                        return SUCOM_ERROR;
                }
                state=WaitTillDone;
                break;
            }

            case WaitTillDone:
                if (line.empty())
                {
                    SetError(SU_ERROR_INCORRECTPASS, "Incorrect password");
                    return SUCOM_NOTAUTHORIZED;
                }

                // Read till we get our personal terminate string...
                if (!line.compare(0, strlen(TermStr), TermStr))
                {
                    //UnReadLine(line);
                    return SUCOM_OK;
                }
                else if (UsingSudo())
                {
                    SetError(SU_ERROR_INCORRECTPASS, "Incorrect password");
                    return SUCOM_NOTAUTHORIZED;
                }
                break;
        }
    }
    return SUCOM_OK;
#endif
}
