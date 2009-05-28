/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.
    
    This file contains code of the KDE project, module kdesu.
    Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
    
    This file contains code from TEShell.C of the KDE konsole.
    Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>
    
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

#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "main/main.h"
#include "main/utils.h"
#include "pseudoterminal.h"

// -------------------------------------
// Pseudo terminal class
// -------------------------------------

// Parts of this class are heavily based on code from KDE's 'kdesu'

CPseudoTerminal::CPseudoTerminal() : m_iPTYFD(0), m_ChildPid(0), m_bEOF(false),
                                     m_bPidExited(false), m_iRetStatus(0)
{  
    CreatePTY();
    GrantPT();
    UnlockPT();
    
    int flags = fcntl(m_iPTYFD, F_GETFL);
    if (flags < 0)
        throw Exceptions::CExFCntl(errno);
    
    if (!(flags & O_NONBLOCK) &&
        (fcntl(m_iPTYFD, F_SETFL, (flags | O_NONBLOCK)) < 0))
        throw Exceptions::CExFCntl(errno);
    
    m_PollData.fd = m_iPTYFD;
    m_PollData.events = POLLIN | POLLHUP;
}

CPseudoTerminal::~CPseudoTerminal()
{
    Abort(false);
      
    if (m_iPTYFD > 0)
    {
        // The process that closes the pty last will recieve a SIGHUP
        // (why?). Ignore the signal temporary...
        sighandler_t prevsig = signal(SIGHUP, SIG_IGN);
        close(m_iPTYFD);
        signal(SIGHUP, prevsig);
    }
}

void CPseudoTerminal::CreatePTY()
{
#if defined(HAVE_GETPT) && defined(HAVE_PTSNAME)
       
    // 1: UNIX98: preferred way
    int pt = getpt();
    
    if (pt == -1)
        throw Exceptions::CExOpenTerm(errno);

    m_iPTYFD = pt;
    m_TTYName = ptsname(m_iPTYFD);
    return;
    
#elif defined(HAVE_OPENPTY)
    
    // 2: BSD interface
    // More preferred than the linux hacks
    char name[30];
    int master_fd, slave_fd;
    if (openpty(&master_fd, &slave_fd, name, 0L, 0L) != -1)
    {
        m_TTYName = name;
        name[5]='p';
        close(slave_fd); // We don't need this yet // Yes, we do.
        m_iPTYFD = master_fd;
        return;
    }
    
    throw Exceptions::CExOpenTerm(errno);
    
#else
    
    // 4: Open terminal device directly
    // 4.1: Try /dev/ptmx first. (Linux w/ Unix98 PTYs, Solaris)
    
    m_iPTYFD = open("/dev/ptmx", O_RDWR);
    if (m_iPTYFD >= 0)
    {
#ifdef HAVE_PTSNAME
        m_TTYName = ::ptsname(m_iPTYFD);
        return;
#elif defined (TIOCGPTN)
        int ptyno;
        if (ioctl(m_iPTYFD, TIOCGPTN, &ptyno) == 0)
        {
            m_TTYName = "/dev/pts/" + ptyno);
            return;
        }
#endif
        close(m_iPTYFD);
    }
    
    m_iPTYFD = 0;
    
    // 4.2: Try /dev/pty[p-e][0-f] (Linux w/o UNIX98 PTY's)
    
    for (const char *c1 = "pqrstuvwxyzabcde"; *c1 != '\0'; c1++)
    {
        for (const char *c2 = "0123456789abcdef"; *c2 != '\0'; c2++)
        {
            std::string PTY = "/dev/pty" + *c1 + *c2;
            m_TTYName = "/dev/tty" + *c1 + *c2;
            if (access(PTY.c_str(), F_OK) < 0)
                goto linux_out;
            m_iPTYFD = open(PTY.c_str(), O_RDWR);
            if (m_iPTYFD >= 0)
                return;
        }
    }
    
linux_out:
    
    m_iPTYFD = 0;
    
    // 4.3: Try /dev/pty%d (SCO, Unixware)
    
    for (int i=0; i<256; i++)
    {
        std::string PTY = "/dev/ptyp" + i;
        m_TTYName = "/dev/ttyp" + i;
        if (access(PTY.c_str(), F_OK) < 0)
            break;
        m_iPTYFD = open(PTY.c_str(), O_RDWR);
        if (m_iPTYFD > 0)
            return;
    }
    
    // Other systems ??
    
    throw Exceptions::CExOpenTerm;
#endif
}

void CPseudoTerminal::GrantPT()
{
#ifdef HAVE_GRANTPT
    
    if (grantpt(m_iPTYFD) == -1)
        throw Exceptions::CExGrantTerm(errno);
    
#elif defined(HAVE_OPENPTY)
    
    // the BSD openpty() interface chowns the devices properly for us,
    // no need to do this at all
    return;
    
#else
    
#error "No grantpt found on your system."
    
    // shut up, gcc
    return;
    
#endif
}

void CPseudoTerminal::UnlockPT()
{
#ifdef HAVE_UNLOCKPT
    
    // (Linux w/ glibc 2.1, Solaris, ...)
    
    if (unlockpt(m_iPTYFD) == -1)
        throw Exceptions::CExUnlockTerm(errno);
    
#elif defined(TIOCSPTLCK)
    
    // Unlock pty (Linux w/ UNIX98 PTY's & glibc 2.0)
    int flag = 0;
    if (ioctl(m_iPTYFD, TIOCSPTLCK, &flag) == -1)
        throw Exceptions::CExUnlockTerm(errno);
    
#else
    
    // Other systems (Linux w/o UNIX98 PTY's, ...)
    
#endif
}

bool CPseudoTerminal::SetupTTY(int fd)
{
    // Reset signal handlers
    for (int sig = 1; sig < NSIG; sig++)
        signal(sig, SIG_DFL);
    signal(SIGHUP, SIG_IGN);
    
    // Close all file handles
    struct rlimit rlp;
    getrlimit(RLIMIT_NOFILE, &rlp);
    for (int i = 0; i < (int)rlp.rlim_cur; i++)
    {
        if (i != fd)
            close(i);
    }
        
    // Create a new session.
    setsid();
      
    // Open slave. This will make it our controlling terminal
    int slave = open(m_TTYName.c_str(), O_RDWR);
    close(fd);
    
    if (slave < 0) 
    {
        fprintf(stderr, "Failed to open slave terminal side: %s\n",
                strerror(errno));
        return false;
    }
        
#if defined(__SVR4) && defined(sun)
    
    // Solaris STREAMS environment.
    // Push these modules to make the stream look like a terminal.
    ioctl(slave, I_PUSH, "ptem");
    ioctl(slave, I_PUSH, "ldterm");
    
#endif
    
#ifdef TIOCSCTTY
    ioctl(slave, TIOCSCTTY, NULL);
#endif
    
    // Connect stdin, stdout and stderr
    dup2(slave, 0);
    dup2(slave, 1);
    dup2(slave, 2);
    
    if (slave > 2) 
        close(slave);
    
    // Disable OPOST processing. Otherwise, '\n' are (on Linux at least)
    // translated to '\r\n'.
    struct termios tio;
    if (tcgetattr(0, &tio) < 0) 
    {
        fprintf(stderr, "Failed to set up slave terminal side (tcgetattr): %s\n",
                strerror(errno));
        return false;
    }
    tio.c_oflag &= ~OPOST;
    if (tcsetattr(0, TCSANOW, &tio) < 0) 
    {
        fprintf(stderr, "Failed to set up slave terminal side (tcsetattr): %s\n",
                strerror(errno));
        return false;
    }   
    
    return true;
}

bool CPseudoTerminal::CheckPidExited(bool block, bool canthrow)
{
    if (m_bPidExited || !m_ChildPid)
        return true;
    
    int state, ret;
    do
    {
        if (canthrow)
            ret = WaitPID(m_ChildPid, &state, (block) ? 0 : WNOHANG);
        else
            ret = waitpid(m_ChildPid, &state, (block) ? 0 : WNOHANG);
    }
    while ((ret == -1) && (errno == EINTR));
        
    if (ret < 0)
    {
        // If we are here canthrow is false
        m_bPidExited = true;
        m_iRetStatus = 1;
        return true;
    }
    
    if (ret == m_ChildPid) 
    {
        m_bPidExited = true;
        
        if (WIFEXITED(state))
            m_iRetStatus = WEXITSTATUS(state);
        
        return true;
    }
    
    return false;
}

void CPseudoTerminal::Exec(const std::string &command)
{   
    m_bEOF = false;
    m_bPidExited = false;
    m_iRetStatus = 0;
    m_ReadBuffer.clear();   
    
    int slave = open(m_TTYName.c_str(), O_RDWR);
    if (slave < 0) 
        throw Exceptions::CExOpenTerm(errno);
    
    m_ChildPid = Fork();
    
    // Parent
    if (m_ChildPid)
    {
        close(slave);
        return;
    }
    
    // Child
    if (!SetupTTY(slave))
        _exit(1);

    if (!m_Path.empty())
        setenv("PATH", m_Path.c_str(), 1);
    
    execl("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);
    _exit(127);
}

bool CPseudoTerminal::HasData()
{
    if (!m_ReadBuffer.empty())
        return true;
    
    if (m_bEOF)
        return false;
    
    int ret = poll(&m_PollData, 1, 10);
    
    if (ret == -1) // Error occured
    {
        if (errno != EINTR) // ignore SIGCHLD
            throw Exceptions::CExPoll(errno);
        else
            return false;
    }
    else if (ret == 0)
        return false;
    else if (((m_PollData.revents & POLLIN) == POLLIN) ||
             ((m_PollData.revents & POLLHUP) == POLLHUP))
        return true;
    
    return false;
}

int CPseudoTerminal::GetRetStatus()
{
    CheckPidExited(true);
    return m_iRetStatus;
}

void CPseudoTerminal::Abort(bool canthrow)
{
    if (m_ChildPid)
    {
        if (kill(-m_ChildPid, SIGTERM) >= 0)
            CheckPidExited(true, canthrow);
        m_ChildPid = 0;
        // Not really required (done in Exec()), but may free some memory
        m_ReadBuffer.clear();
    }
}

CPseudoTerminal::EReadStatus CPseudoTerminal::ReadLine(std::string &out)
{
    TSTLStrSize nl = m_ReadBuffer.find("\n");
    if (nl != std::string::npos)
    {
        out = m_ReadBuffer.substr(0, nl);
        m_ReadBuffer.erase(0, nl+1);
        return READ_LINE;
    }
    
    const int bufsize = 512;
    char buffer[bufsize+1];
    
    int readret = read(m_iPTYFD, &buffer, bufsize);
    
    if (readret <= 0)
    {
        m_bEOF = true;
        out = m_ReadBuffer;
        m_ReadBuffer.clear();
        
        // Linux gives EIO when slave is closed
        if ((readret == -1) && (errno != EIO))
            throw Exceptions::CExReadTerm(errno);
        
        return READ_EOF;
    }
    
    buffer[readret] = 0;
    m_ReadBuffer += buffer;
    
    nl = m_ReadBuffer.find("\n");
    if (nl != std::string::npos)
    {
        out = m_ReadBuffer.substr(0, nl);
        m_ReadBuffer.erase(0, nl+1);
        return READ_LINE;
    }
    
    // No newline found yet
    return READ_AGAIN;
}

CPseudoTerminal::operator void *()
{
    static int ret;
    bool check = CheckPidExited(false); // Need to do this here
    if (m_bEOF || !m_iPTYFD || !m_ChildPid || (!HasData() && check))
        return NULL;
    return &ret;
}
