/*
    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

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

// Low level terminal code

#include <main.h>

using namespace LIBSU;

int CLibSU::CreatePT()
{

#if defined(HAVE_GETPT) && defined(HAVE_PTSNAME)

    // 1: UNIX98: preferred way
    m_iPTYFD = ::getpt();
    m_szTTYName = ::ptsname(m_iPTYFD);
    return m_iPTYFD;

#elif defined(HAVE_OPENPTY)

    // 2: BSD interface
    // More preferred than the linux hacks
    char name[30];
    int master_fd, slave_fd;
    if (openpty(&master_fd, &slave_fd, name, 0L, 0L) != -1)
    {
        m_szTTYName = name;
        name[5]='p';
        m_szPTYName = name;
        close(slave_fd); // We don't need this yet // Yes, we do.
        m_iPTYFD = master_fd;
        log("Created PT: TTYName: %s, PTYName: %s, FD: %d\n", m_szTTYName.c_str(), m_szPTYName.c_str(), master_fd);
        return m_iPTYFD;
    }
    m_iPTYFD = -1;
    SetError(SU_ERROR_INTERNAL, "Opening pty failed");
    return -1;

#elif defined(HAVE__GETPTY)

    // 3: Irix interface
    int master_fd;
    m_szTTYName = _getpty(&master_fd,O_RDWR,0600,0);
    if (m_szTTYName)
        m_iPTYFD = master_fd;
    else
    {
        m_iPTYFD = -1;
        SetError(SU_ERROR_INTERNAL, "Opening pty failed. error %d", errno);
    }
    return m_iPTYFD;

#else

    // 4: Open terminal device directly
    // 4.1: Try /dev/ptmx first. (Linux w/ Unix98 PTYs, Solaris)

    m_iPTYFD = open("/dev/ptmx", O_RDWR);
    if (m_iPTYFD >= 0)
    {
        m_szPTYName = "/dev/ptmx";
#ifdef HAVE_PTSNAME
        m_szTTYName = ::ptsname(m_iPTYFD);
        return m_iPTYFD;
#elif defined (TIOCGPTN)
        int ptyno;
        if (ioctl(m_iPTYFD, TIOCGPTN, &ptyno) == 0)
        {
            m_szTTYName = "/dev/pts/" + ptyno);
            return m_iPTYFD;
        }
#endif
        close(m_iPTYFD);
    }

    // 4.2: Try /dev/pty[p-e][0-f] (Linux w/o UNIX98 PTY's)

    for (const char *c1 = "pqrstuvwxyzabcde"; *c1 != '\0'; c1++)
    {
        for (const char *c2 = "0123456789abcdef"; *c2 != '\0'; c2++)
        {
            m_szPTYName = "/dev/pty" + *c1 + *c2;
            m_szTTYName = "/dev/tty" + *c1 + *c2;
            if (access(m_szPTYName.c_str(), F_OK) < 0)
                goto linux_out;
            m_iPTYFD = open(m_szPTYName.c_str(), O_RDWR);
            if (m_iPTYFD >= 0)
                return m_iPTYFD;
        }
    }
linux_out:

    // 4.3: Try /dev/pty%d (SCO, Unixware)

    for (int i=0; i<256; i++)
    {
        m_szPTYName = "/dev/ptyp" + i;
        m_szTTYName = "/dev/ttyp" + i;
        if (access(m_szPTYName.c_str(), F_OK) < 0)
            break;
        m_iPTYFD = open(m_szPTYName.c_str(), O_RDWR);
        if (m_iPTYFD >= 0)
            return m_iPTYFD;
    }

    // Other systems ??
    m_iPTYFD = -1;
    SetError(SU_ERROR_INTERNAL, "Unknown system or all methods failed");
    return -1;

#endif // HAVE_GETPT && HAVE_PTSNAME

}


int CLibSU::GrantPT()
{
    if (m_iPTYFD < 0)
        return -1;

#ifdef HAVE_GRANTPT

    return ::grantpt(m_iPTYFD);

#elif defined(HAVE_OPENPTY)

    // the BSD openpty() interface chowns the devices properly for us,
    // no need to do this at all
    return 0;

#else

// UNDONE

#error "No grantpt found on your system."

    // shut up, gcc
    return 0;

#endif // HAVE_GRANTPT
}


/**
 * Unlock the pty. This allows connections on the slave side.
 */

int CLibSU::UnlockPT()
{
    if (m_iPTYFD < 0)
        return -1;

#ifdef HAVE_UNLOCKPT

    // (Linux w/ glibc 2.1, Solaris, ...)

    return ::unlockpt(m_iPTYFD);

#elif defined(TIOCSPTLCK)

    // Unlock pty (Linux w/ UNIX98 PTY's & glibc 2.0)
    int flag = 0;
    return ioctl(m_iPTYFD, TIOCSPTLCK, &flag);

#else

    // Other systems (Linux w/o UNIX98 PTY's, ...)
    return 0;

#endif

}

int CLibSU::SetupTTY(int fd)
{
    // Reset signal handlers
    for (int sig = 1; sig < NSIG; sig++)
        signal(sig, SIG_DFL);
    signal(SIGHUP, SIG_IGN);

    // Close all file handles
    struct rlimit rlp;
    getrlimit(RLIMIT_NOFILE, &rlp);
    for (int i = 0; i < (int)rlp.rlim_cur; i++)
        if (i != fd) close(i); 

    // Create a new session.
    setsid();

    // Open slave. This will make it our controlling terminal
    int slave = open(m_szTTYName.c_str(), O_RDWR);
    if (slave < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "Could not open slave side: %s", perror);
        return -1;
    }
    close(fd);

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
    dup2(slave, 0); dup2(slave, 1); dup2(slave, 2);
    if (slave > 2) 
        close(slave);

    // Disable OPOST processing. Otherwise, '\n' are (on Linux at least)
    // translated to '\r\n'.
    struct termios tio;
    if (tcgetattr(0, &tio) < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "tcgetattr(): %s", perror);
        return -1;
    }
    tio.c_oflag &= ~OPOST;
    if (tcsetattr(0, TCSANOW, &tio) < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "tcsetattr(): %s", perror);
        return -1;
    }

    return 0;
}

void CLibSU::WriteLine(const std::string &line, bool AddLine)
{
    if (!line.empty())
        write(m_iPTYFD, line.c_str(), line.length());
    if (AddLine)
        write(m_iPTYFD, "\n", 1);
}


void CLibSU::UnReadLine(const std::string &line, bool AddLine)
{
    std::string tmp = line;
    if (AddLine)
        tmp += '\n';
    if (!tmp.empty())
        m_szInBuf.insert(0, tmp);
}

int CLibSU::WaitSlave()
{
    int slave = open(m_szTTYName.c_str(), O_RDWR);
    if (slave < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "Could not open slave tty");
        return -1;
    }

    struct termios tio;
    while (1) 
    {
        if (!CheckPid(m_iPid))
        {
            close(slave);
            return -1;
        }
        if (tcgetattr(slave, &tio) < 0) 
        {
            SetError(SU_ERROR_INTERNAL, "tcgetattr(): %s", perror);
            close(slave);
            return -1;
        }
        if (tio.c_lflag & ECHO) 
        {
            WaitMS(slave,100);
            continue;
        }
        break;
    }
    close(slave);
    return 0;
}

int CLibSU::WaitMS(int fd, int ms)
{
    struct timeval tv;
    tv.tv_sec = 0; 
    tv.tv_usec = 1000*ms;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    return select(fd+1, &fds, 0L, 0L, &tv);
}
