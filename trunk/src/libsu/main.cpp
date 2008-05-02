/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of libsu.

    This file contains code of the KDE project, module kdesu.
    Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>

    This file contains code from TEShell.C of the KDE konsole.
    Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de> 

    libsu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    libsu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libsu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <main.h>
#include <ctype.h>
#include <time.h>

static const char *TermStr = "Im Done Now :)"; // Lets just hope another program doesn't output this ;)

namespace LIBSU
{

std::string RunnerPath = "./";
ESuType SuType = TYPE_UNKNOWN;

void SetDefSuType()
{
    if (GetSuType() == TYPE_MAYBESUDO)
        SetSuType(TYPE_SUDO);
    else if (GetSuType() == TYPE_MAYBESU)
        SetSuType(TYPE_SU);
}

CLibSU::CLibSU(bool Disable0Core) : m_iPTYFD(0), m_iPid(0), m_bTerminal(false),
                                    m_iLastRET(0), m_szUser("root"), m_szPath("/bin:/usr/bin"),
                                    m_eError(SU_ERROR_NONE), m_pThinkFunc(NULL),
                                    m_pOutputFunc(NULL), m_pCustomThinkData(NULL), m_pCustomOutputData(NULL)
{
    if (!Disable0Core)
    {
        // Set core dump size to 0 because we will have
        // root's password in memory.
        struct rlimit rlim;
        rlim.rlim_cur = rlim.rlim_max = 0;
        if (setrlimit(RLIMIT_CORE, &rlim))
        {
            SetError(SU_ERROR_INTERNAL, "rlimit(): %s", strerror(errno));
        }
    }
}

CLibSU::CLibSU(const char *command, const char *user, const char *path,
               bool Disable0Core) : m_iPTYFD(0), m_iPid(0), m_bTerminal(true), m_iLastRET(0),
                                    m_szCommand(command), m_szUser(user), m_szPath(path),
                                    m_eError(SU_ERROR_NONE), m_pThinkFunc(NULL), m_pOutputFunc(NULL),
                                    m_pCustomThinkData(NULL), m_pCustomOutputData(NULL)
{
    if (!user || !user[0]) m_szUser = "root";
    
    if (!Disable0Core)
    {
        // Set core dump size to 0 because we will have
        // root's password in memory.
        struct rlimit rlim;
        rlim.rlim_cur = rlim.rlim_max = 0;
        if (setrlimit(RLIMIT_CORE, &rlim))
        {
            SetError(SU_ERROR_INTERNAL, "rlimit(): %s", strerror(errno));
        }
    }
}

void CLibSU::SetCommand(const std::string &command)
{
    m_szCommand.clear();
    
    // As the command is executed with single quotes (sh -c 'cmd') all the single quotes need to be transformed to use them.
    // For this we use replace them by following string which concats single quotes.
    const char *quotehack = "\'\"\'\"\'";
    const std::string::size_type length = command.length();
    std::string::size_type ind = 0;
    
    while (ind < length)
    {
        if (command[ind] == '\'')
        {
            m_szCommand += quotehack;
        }
        else
        {
            m_szCommand += command[ind];
        }
        ind++;
    }
}

const char *CLibSU::GetSuBin(ESuType type) const
{
    if ((type == TYPE_MAYBESUDO) || (type == TYPE_SUDO))
    {
        if (FileExists("/usr/bin/sudo"))
            return "/usr/bin/sudo";
        else if (FileExists("/bin/sudo"))
            return "/bin/sudo";
    }
    else if ((type == TYPE_MAYBESU) || (type == TYPE_SU))
    {
        if (FileExists("/usr/bin/su"))
            return "/usr/bin/su";
        else if (FileExists("/bin/su"))
            return "/bin/su";
    }
    
    return NULL;
}

void CLibSU::ConstructCommand(std::string &command, std::vector<std::string> &args, const std::string &runcmd,
                              const std::vector<std::string> &runargs)
{
    if (GetSuType() == TYPE_UNKNOWN)
    {
        if (GetSuBin(TYPE_SUDO)) // Always try sudo first (when unconfigured it may work the pw of target user)
            SetSuType(TYPE_MAYBESUDO);
        else
            SetSuType(TYPE_MAYBESU);
    }
    
    command = GetSuBin(GetSuType());
    
    if (UsingSudo())
        args.push_back("-u");
    
    args.push_back(m_szUser);
    
    if (UsingSudo())
    {
        args.push_back("-s");
        args.push_back("--");
        args.push_back("-c");
    }
    else
        args.push_back("-c");

    args.push_back(std::string("printf \"") + TermStr + "\\n\" ;" + runcmd); // Insert our magic terminate indicator to command
    
    if (!UsingSudo())
        args.push_back("-");
    
    args.insert(args.end(), runargs.begin(), runargs.end());
}

int CLibSU::Exec(const std::string &command, const std::vector<std::string> &args)
{
    std::string realcmd;
    std::vector<std::string> realargs;
    ConstructCommand(realcmd, realargs, command, args);
    
    if (realcmd.empty())
    {
        SetError(SU_ERROR_SUNOTFOUND, "Couldn't find suitable binary.");
        return -1;
    }

    log("Running '%s'\n", realcmd.c_str());

    if (m_iPTYFD)
    {
        close(m_iPTYFD);
        m_iPTYFD = 0;
    }
    
    if (CreatePT() < 0)
        return -1;
        
    if ((GrantPT() < 0) || (UnlockPT() < 0))
    {
        SetError(SU_ERROR_INTERNAL, "Master setup failed: %s", strerror(errno));
        return -1;
    }
    
    m_szInBuf = "";
    
    // Open the pty slave before forking. See SetupTTY()
    int slave = open(m_szTTYName.c_str(), O_RDWR);
    if (slave < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "Could not open slave pty");
        return -1;
    }

    m_iPid = fork();
           
    if (m_iPid == -1) 
    {
        SetError(SU_ERROR_INTERNAL, "fork(): %s ", strerror(errno));
        return -1;
    } 

    // Parent
    if (m_iPid)
    {
        close(slave);
        return 0;
    }

    // Child
    if (SetupTTY(slave) < 0)
    {
        SetError(SU_ERROR_INTERNAL, "Couldn't set up TTY");
        return -1;
    }

    if (UsingSudo())
        system("sudo -k"); // Forget about any previous sessions (this is needed for TestSU() and NeedPassword())

    // From now on, terminal output goes through the tty.

    char **argp = (char **)malloc((realargs.size()+2)*sizeof(char *));
    int i = 0;

    argp[i] = strdup(realcmd.c_str());
    i++;

    log("args: ");
    for (std::vector<std::string>::const_iterator it=realargs.begin(); it!=realargs.end(); it++)
    {
        argp[i] = strdup(it->c_str());
        log("%s ", argp[i]);
        i++;
    }
    
    log("\n");
    argp[i] = 0L;
    
    setenv("PATH", m_szPath.c_str(), 1);
    execv(realcmd.c_str(), (char * const *)argp);
    SetError(SU_ERROR_EXECUTE, "execv(\"%s\"): %s", m_szPath.c_str(), strerror(errno));
    return -1;
}

std::string CLibSU::ReadLine(bool block)
{
    std::string::size_type pos;
    std::string ret;

    if (!m_szInBuf.empty())
    {
        pos = m_szInBuf.find('\n');
        if (pos == std::string::npos)
        {
            ret = m_szInBuf;
            m_szInBuf.clear();
        }
        else
        {
            ret = m_szInBuf.substr(0, pos+1);
            m_szInBuf.erase(0, pos+1);
        }
        log("ret in ReadLine: %s(%d, buffered)\nBuffer: %s\n", ret.c_str(), ret.length(), m_szInBuf.c_str());
        return ret;
    }

    int flags = fcntl(m_iPTYFD, F_GETFL);
    if (flags < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "fcntl(F_GETFL): %s", strerror(errno));
        return ret;
    }
    int oflags = flags;
    if (block)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;

    if ((flags != oflags) && (fcntl(m_iPTYFD, F_SETFL, flags) < 0))
    {
       // We get an error here when the child process has closed 
       // the file descriptor already.
       return ret;
    }
    
    int nbytes;
    char buf[256];
    while (1) 
    {
        nbytes = read(m_iPTYFD, buf, 255);

        log("Read %d bytes\n", nbytes);

        if (nbytes == -1) 
        {
            if (errno == EINTR)
                continue;
            else
            {
                log("Read had an error: %s", strerror(errno));
                break;
            }
        }
        if (nbytes == 0)
            break;        // eof

        buf[nbytes] = '\000';
        log("ReadLine: %s\n", buf);
        m_szInBuf += buf;

        pos = m_szInBuf.find('\n');
        if (pos == std::string::npos) 
        {
            ret = m_szInBuf;
            m_szInBuf.clear();
        }
        else 
        {
            ret = m_szInBuf.substr(0, pos+1);
            m_szInBuf = m_szInBuf.substr(pos+1);
        }
        break;
    }
    
    log("ret in ReadLine: %s(%d, direct)\nBuffer: %s\n", ret.c_str(), ret.length(), m_szInBuf.c_str());
    return ret;
}

int CLibSU::WaitForChild()
{
    int retval = 1;

    fd_set fds;
    FD_ZERO(&fds);

    // HACK: Wait 0.2 sec to make sure that ReadLine() won't return unfinished lines
    timespec tsec = { 0, 200000000 };
    nanosleep(&tsec, NULL);

    while (true)
    {
        if (m_pThinkFunc)
            (m_pThinkFunc)(m_pCustomThinkData);

        int ret = 0;
        if (m_szInBuf.empty()) // HACK: Shouldn't be here
        {
            FD_ZERO(&fds);
            FD_SET(m_iPTYFD, &fds);
            timeval tv = { 0, 10 };
            
            ret = select(m_iPTYFD+1, &fds, 0L, 0L, &tv);
            if (ret == -1) 
            {
                if (errno != EINTR) 
                {
                    SetError(SU_ERROR_INTERNAL, "select(): %s", strerror(errno));
                    return -1;
                }
                ret = 0;
            }
        }
        else
            ret = 1;
        
        if (ret) // There is input available
        {
            std::string line = ReadLine(false);
            while (!line.empty())
            {
                if (!m_szExit.empty() && (line == m_szExit))
                    kill(-m_iPid, SIGTERM);
                if (m_bTerminal)
                {
                    fputs(line.c_str(), stdout);
                }
                
                if (m_pThinkFunc)
                    (m_pThinkFunc)(m_pCustomThinkData);

                std::string::size_type pos = line.find('\n');
                if (pos != std::string::npos)
                {
                    if (m_pOutputFunc)
                        (m_pOutputFunc)(line.substr(0, pos+1).c_str(), m_pCustomOutputData);
                    
                    line.erase(0, pos+1);
                }
                
                line += ReadLine(false);
            }
            
            if (!line.empty() && m_pOutputFunc)
                (m_pOutputFunc)(line.c_str(), m_pCustomOutputData);
        }

        ret = CheckPidExited(m_iPid);
        log("CheckPidExited: %d\n", ret);
        
        if (ret == PID_ERROR)
        {
            if (errno == ECHILD) retval = 0;
            else retval = 1;
            break;
        }
        else if (ret == PID_KILLED)
        {
            retval = 0;
            break;
        }
        else if (ret == PID_NOTEXITED)
        {
            // keep checking
        }
        else
        {
            retval = ret;
            break;
        }
    }
    
    close(m_iPTYFD);
    m_iPTYFD = 0;
    
    return retval;
}

int CLibSU::CheckPidExited(pid_t pid)
{
    int state, ret;
    ret = waitpid(pid, &state, WNOHANG);

    if (ret < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "waitpid(): %s", strerror(errno));
        return PID_ERROR;
    }
    if (ret == pid) 
    {
        if (WIFEXITED(state))
            return WEXITSTATUS(state);
        return PID_KILLED;
    }

    return PID_NOTEXITED;
}

void CLibSU::SetError(CLibSU::ESuErrors errtype, const char *msg, ...)
{
    char *buffer;
    va_list v;
    
    va_start(v, msg);
    vasprintf(&buffer, msg, v);
    va_end(v);
    
    m_eError = errtype;
    
    m_szErrorMsg = buffer;
    log(buffer);
    
    free(buffer);
}

int CLibSU::TalkWithSU(const char *password)
{	
    enum { WaitForPrompt, CheckStar, WaitTillDone } state = WaitForPrompt;
    int colon;
    std::string::size_type i, j;

    std::string line;
    while (true)
    {
        line = ReadLine(); 
        
        log("Read Line: %s\nState: %d\n", line.c_str(), state);

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
}

bool CLibSU::NeedPassword()
{
    std::string command;
    std::vector<std::string> args;

    if (Exec(command, args) < 0)
    {
        return false;
    }
    
    ESuComErrors ret = (ESuComErrors) TalkWithSU(0L);
    log("SU COM(need pw): %d\n", ret);
    
    if (ret == SUCOM_NULLPASS)
    {
        if (kill(-m_iPid, SIGKILL) >= 0)
            WaitForChild();
        return true;
    }
    else if (ret == SUCOM_NOTAUTHORIZED)
    {
        if (kill(-m_iPid, SIGKILL) >= 0)
            WaitForChild();

        if (GetSuType() == TYPE_MAYBESUDO)
        {
            SetSuType(TYPE_MAYBESU); // Try again with su
            return NeedPassword();
        }
        else
            SetSuType(TYPE_UNKNOWN);
        
        return false;
    }

    SetDefSuType();
    
    WaitForChild();
    return false;
}

bool CLibSU::TestSU(const char *password)
{
    std::string command;
    std::vector<std::string> args;

    if (Exec(command, args) < 0)
    {
        return false;
    }
    
    ESuComErrors ret = (ESuComErrors) TalkWithSU(password);
    log("SU COM(test): %d\n", ret);
    
    if (ret == SUCOM_ERROR) 
    {
        SetError(SU_ERROR_INTERNAL, "Conversation with su/sudo failed");
        return false;
    }
    
    if (ret == SUCOM_NULLPASS)
    {
        if (kill(-m_iPid, SIGKILL) >= 0) WaitForChild();
        return false;
    }

    if (ret == SUCOM_NOTAUTHORIZED)
    {
        if (kill(-m_iPid, SIGKILL) >= 0)
            WaitForChild();
        
        if (GetSuType() == TYPE_MAYBESUDO)
        {
            SetSuType(TYPE_MAYBESU); // Try again with su
            log("Trying su after sudo\n");
            if (TestSU(password))
            {
                SetSuType(TYPE_SU);
                return true;
            }
            else
                SetSuType(TYPE_UNKNOWN);
        }
        
        return false;
    }

    SetDefSuType();
    
    WaitForChild();
    return true;
}

bool CLibSU::ExecuteCommand(const char *password, bool removepass)
{
    std::string command;
    std::vector<std::string> args;
    
    char *cmdfmt = FormatText("%s/surunner \'%s\'", GetRunnerPath().c_str(), m_szCommand.c_str());
    
    if (Exec(cmdfmt, args) < 0)
    {
        free(cmdfmt);
        return false;
    }
    
    free(cmdfmt);
    
    ESuComErrors ret = (ESuComErrors) TalkWithSU(password);
    log("SU COM: %d\n", ret);
    
    if (ret == SUCOM_ERROR) 
    {
        SetError(SU_ERROR_INTERNAL, "Conversation with su/sudo failed");
        return false;
    }
    
    if (ret == SUCOM_NULLPASS)
    {
        if (kill(-m_iPid, SIGKILL) >= 0)
            WaitForChild();
        return false;
    }

    if (removepass && password)
    {
        char *ptr = const_cast<char *>(password);
        size_t len = strlen(password);
        for (size_t i=0; i<len; i++)
            ptr[i] = '\000';
    }

    if (ret == SUCOM_NOTAUTHORIZED)
    {
        if (kill(-m_iPid, SIGKILL) >= 0)
            WaitForChild();
        
        if (GetSuType() == TYPE_MAYBESUDO)
        {
            SetSuType(TYPE_MAYBESU); // Try again with su
            if (ExecuteCommand(password, removepass))
                SetSuType(TYPE_SU);
            else
            {
                SetSuType(TYPE_UNKNOWN);
                return false;
            }
        }
        else
            return false;
    }

    SetDefSuType();
    
    m_iLastRET = WaitForChild();
    if (m_iLastRET == 127)
    {
        SetError(SU_ERROR_EXECUTE, "Couldn't execute \'%s\'", m_szCommand.c_str());
        return false;
    }
    else
        log("Child exited with status %d\n", m_iLastRET);
    
    return true;
}

}
