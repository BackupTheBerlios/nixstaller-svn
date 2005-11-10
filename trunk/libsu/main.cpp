#include <main.h>
#include <stdarg.h>

const char *TermStr = "I'm Done Now :)"; // Lets just hope other program don't output this ;)

CLibSU::CLibSU(bool Disable0Core) : m_iPTYFD(0), m_iPid(0), m_bTerminal(true), m_szUser("root"), m_szPath("/bin:/usr/bin"),
                                    m_eError(SU_ERROR_NONE), m_pOutputFunc(NULL), m_pCustomData(NULL)
{
    if (!Disable0Core)
    {
        // Set core dump size to 0 because we will have
        // root's password in memory.
        struct rlimit rlim;
        rlim.rlim_cur = rlim.rlim_max = 0;
        if (setrlimit(RLIMIT_CORE, &rlim))
        {
            SetError(SU_ERROR_INTERNAL, "rlimit(): %s\n", strerror(errno));
            exit(1);
        }
    }
}

CLibSU::CLibSU(const char *command, const char *user, const char *path,
               bool Disable0Core) : m_iPTYFD(0), m_iPid(0), m_bTerminal(true), m_szCommand(command), m_szUser(user),
                                    m_szPath(path), m_eError(SU_ERROR_NONE), m_pOutputFunc(NULL), m_pCustomData(NULL)
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
            SetError(SU_ERROR_INTERNAL, "rlimit(): %s\n", strerror(errno));
            exit(1);
        }
    }
}

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
    if (openpty(&master_fd, &slave_fd, name, 0L, 0L) != -1)  {
        m_szTTYName = name;
        name[5]='p';
        m_szPTYName = name;
        close(slave_fd); // We don't need this yet // Yes, we do.
        m_iPTYFD = master_fd;
        log("Created PT: TTYName: %s, PTYName: %s, FD: %d\n", m_szTTYName.c_str(), m_szPTYName.c_str(), master_fd);
        return m_iPTYFD;
    }
    m_iPTYFD = -1;
    SetError(SU_ERROR_INTERNAL, "Opening pty failed.\n");
    return -1;

#elif defined(HAVE__GETPTY)
    // 3: Irix interface
    int master_fd;
    m_szTTYName = _getpty(&master_fd,O_RDWR,0600,0);
    if (m_szTTYName)
        m_iPTYFD = master_fd;
    else{
        m_iPTYFD = -1;
        SetError(SU_ERROR_INTERNAL, "Opening pty failed. error %d\n", errno);
    }
    return m_iPTYFD;

#else

    // 4: Open terminal device directly
    // 4.1: Try /dev/ptmx first. (Linux w/ Unix98 PTYs, Solaris)

    m_iPTYFD = open("/dev/ptmx", O_RDWR);
    if (m_iPTYFD >= 0) {
        m_szPTYName = "/dev/ptmx";
#ifdef HAVE_PTSNAME
        m_szTTYName = ::ptsname(m_iPTYFD);
        return m_iPTYFD;
#elif defined (TIOCGPTN)
        int ptyno;
        if (ioctl(m_iPTYFD, TIOCGPTN, &ptyno) == 0) {
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
    SetError(SU_ERROR_INTERNAL, "Unknown system or all methods failed.\n");
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

    // konsole_grantpty only does /dev/pty??
    if (m_szPTYName.left(8) != "/dev/pty")
        return 0;

    // Use konsole_grantpty:
    if (KStandardDirs::findExe("konsole_grantpty").isEmpty())
    {
        kdError(900) << k_lineinfo << "konsole_grantpty not found.\n";
        return -1;
    }

    // As defined in konsole_grantpty.c
    const int pty_fileno = 3;

    pid_t pid;
    if ((pid = fork()) == -1)
    {
        kdError(900) << k_lineinfo << "fork(): " << perror << "\n";
        return -1;
    }

    if (pid)
    {
        // Parent: wait for child
        int ret;
        waitpid(pid, &ret, 0);
        if (WIFEXITED(ret) && !WEXITSTATUS(ret))
            return 0;
        kdError(900) << k_lineinfo << "konsole_grantpty returned with error: "
                     << WEXITSTATUS(ret) << "\n";
        return -1;
    } else
    {
        // Child: exec konsole_grantpty
        if (m_iPTYFD != pty_fileno && dup2(m_iPTYFD, pty_fileno) < 0)
            _exit(1);
        execlp("konsole_grantpty", "konsole_grantpty", "--grant", (void *)0);
        kdError(900) << k_lineinfo << "exec(): " << perror << "\n";
        _exit(1);
    }

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

int CLibSU::Exec(const std::string &command, const std::list<std::string> &args)
{
    log("Running '%s'\n", command.c_str());

#if 0
    // Check if command exists
    bool fexists = false;
    std::string tmp_path = m_szPath;
    while(1)
    {
        std::string::size_type pos = tmp_path.find(':');
        if (FileExists(tmp_path.substr(0, (pos==std::string::npos)?pos:pos) + '/' + m_szCommand))
        {
            fexists = true;
            break;
        }
        else log("%s doesn't exist\n", std::string(tmp_path.substr(0, (pos==std::string::npos)?pos:pos) + '/' + m_szCommand).c_str());
        if (pos == std::string::npos) break;
        tmp_path.erase(0, pos+1);
    }
    
    if (!fexists)
    {
        SetError(SU_ERROR_INTERNAL, "Couldn't find executable\n");
        return -1;
    }
#endif

    if (CreatePT() < 0)
        return -1;
        
    if ((GrantPT() < 0) || (UnlockPT() < 0)) 
    {
        SetError(SU_ERROR_INTERNAL, "Master setup failed.\n");
        return -1;
    }
    
    m_szInBuf = "";
    
    // Open the pty slave before forking. See SetupTTY()
    int slave = open(m_szTTYName.c_str(), O_RDWR);
    if (slave < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "Could not open slave pty.\n");
        return -1;
    }

    m_iPid = fork();
           
    if (m_iPid == -1) 
    {
        SetError(SU_ERROR_INTERNAL, "fork(): %s\n ", perror);
        return -1;
    } 

    log("Pid: %d\n", m_iPid);
    
    // Parent
    if (m_iPid) 
    {
        close(slave);
        return 0;
    }

    log("Child running...\n");
    
    // Child
    if (SetupTTY(slave) < 0)
    {
        SetError(SU_ERROR_INTERNAL, "Couldn't set up TTY\n");
        return -1;
    }

    // From now on, terminal output goes through the tty.

    char **argp = (char **)malloc((args.size()+2)*sizeof(char *));
    int i = 0;

    argp[i] = new char[command.length()+1];
    command.copy(argp[i], std::string::npos);
    argp[i][command.length()] = 0;
    i++;

    log("args: ");
    for (std::list<std::string>::const_iterator it=args.begin(); it!=args.end(); it++)
    {
        argp[i] = new char[it->length()+1];
        it->copy(argp[i], std::string::npos);
        argp[i][it->length()] = 0;
        log("%s ", argp[i]);
        i++;
    }
    
    log("\n");
    argp[i] = 0L;
    
    setenv("PATH", m_szPath.c_str(), 1);
    execv(command.c_str(), (char * const *)argp);
    SetError(SU_ERROR_EXECUTE, "execv(\"%s\"): %s\n", m_szPath.c_str(), perror);
    return -1; // Shut up compiler. Never reached.
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
        SetError(SU_ERROR_INTERNAL, "Could not open slave side: %s\n", perror);
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
        SetError(SU_ERROR_INTERNAL, "tcgetattr(): %s\n", perror);
        return -1;
    }
    tio.c_oflag &= ~OPOST;
    if (tcsetattr(0, TCSANOW, &tio) < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "tcsetattr(): %s\n", perror);
        return -1;
    }

    return 0;
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
        } else
        {
            ret = m_szInBuf.substr(0, pos+1);
            m_szInBuf = m_szInBuf.substr(pos+1);
        }
        log("ret(1) in ReadLine: %s(%d)\n", ret.c_str(), ret.length());
        return ret;
    }

    int flags = fcntl(m_iPTYFD, F_GETFL);
    if (flags < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "fcntl(F_GETFL): %s\n", perror);
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
            else break;
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
        } else 
        {
            ret = m_szInBuf.substr(0, pos+1);
            m_szInBuf = m_szInBuf.substr(pos+1);
        }
        break;
    }

    log("ret(2) in ReadLine: %s(%d)\n", ret.c_str(), ret.length());
    return ret;
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
        SetError(SU_ERROR_INTERNAL, "Could not open slave tty.\n");
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
            SetError(SU_ERROR_INTERNAL, "tcgetattr(): %s\n", perror);
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

int CLibSU::WaitForChild()
{
    int retval = 1;

    fd_set fds;
    FD_ZERO(&fds);

    while (1) 
    {
        FD_SET(m_iPTYFD, &fds);
        int ret = select(m_iPTYFD+1, &fds, 0L, 0L, 0L);
        if (ret == -1) 
        {
            if (errno != EINTR) 
            {
                SetError(SU_ERROR_INTERNAL, "select(): %s\n", perror);
                return -1;
            }
            ret = 0;
        }

        if (ret) 
        {
            std::string line = ReadLine(false);
            while (!line.empty()) 
            {
                if (!m_szExit.empty() && (line == m_szExit))
                    kill(m_iPid, SIGTERM);
                if (m_bTerminal)
                {
                    fputs(line.c_str(), stdout);
                    //fputc('\n', stdout);
                }
                if (m_pOutputFunc) (m_pOutputFunc)(line.c_str(), m_pCustomData);
                line = ReadLine(false);
            }
        }

        ret = CheckPidExited(m_iPid);
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
    return retval;
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

int CLibSU::CheckPidExited(pid_t pid)
{
    int state, ret;
    ret = waitpid(pid, &state, WNOHANG);

    if (ret < 0) 
    {
        SetError(SU_ERROR_INTERNAL, "waitpid(): %s\n", perror);
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
    char buffer[1024];
    va_list v;
    
    m_eError = errtype;
    
    va_start(v, msg);
        vsprintf(buffer, msg, v);
    va_end(v);
    
    m_szErrorMsg = buffer;
    log(buffer);
}

int CLibSU::TalkWithSU(const char *password)
{	
    enum { WaitForPrompt, CheckStar, WaitTillDone } state = WaitForPrompt;
    int colon;
    unsigned i, j;

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
                    SetError(SU_ERROR_INCORRECTUSER, "Incorrect user\n");
                    return SUCOM_NOTAUTHORIZED;
                }
                
                // In case no password is needed.
                if (line == TermStr)
                {
                    UnReadLine(line);
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
                        SetError(SU_ERROR_INCORRECTPASS, "Incorrect password\n");
                        return SUCOM_NULLPASS;
                    }
                    if (!CheckPid(m_iPid))
                    {
                        SetError(SU_ERROR_INTERNAL, "su has exited while waiting for pwd.\n");
                        return SUCOM_ERROR;
                    }
                    if ((WaitSlave() == 0) && CheckPid(m_iPid))
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
                    SetError(SU_ERROR_INCORRECTPASS, "Incorrect password\n");
                    return SUCOM_NOTAUTHORIZED;
                }

                // Read till we get our personal terminate string...
                if (line == TermStr)
                {
                    //UnReadLine(line);
                    return SUCOM_OK;
                }
                break;
        }
    }
    return SUCOM_OK;
}

bool CLibSU::NeedPassword()
{
    std::list<std::string> args;

    args.push_back(m_szUser);
    args.push_back("-c");
    args.push_back(std::string("printf \"") + TermStr + "\""); // Insert our magic terminate indicator to command
    args.push_back("-");

    std::string command;
    if (FileExists("/usr/bin/su")) command = "/usr/bin/su";
    else if (FileExists("/bin/su")) command = "/bin/su";

    if (command.empty())
    {
        SetError(SU_ERROR_SUNOTFOUND, "Couldn't find su\n");
        return false;
    }
    
    if (Exec(command, args) < 0)
    {
	return false;
    }
    
    ESuComErrors ret = (ESuComErrors) TalkWithSU(0L);
    log("SU COM(need pw): %d\n", ret);
    
    if (ret == SUCOM_NULLPASS)
    {
        if (kill(m_iPid, SIGKILL) >= 0) WaitForChild();
        return true;
    }

    WaitForChild();
    return false;
}

bool CLibSU::TestSU(const char *password)
{
    std::list<std::string> args;

    args.push_back(m_szUser);
    args.push_back("-c");
    args.push_back(std::string("printf \"") + TermStr + "\""); // Insert our magic terminate indicator to command
    args.push_back("-");

    std::string command;
    if (FileExists("/usr/bin/su")) command = "/usr/bin/su";
    else if (FileExists("/bin/su")) command = "/bin/su";

    if (command.empty())
    {
        SetError(SU_ERROR_SUNOTFOUND, "Couldn't find su\n");
        return false;
    }
    
    if (Exec(command, args) < 0)
    {
	return false;
    }
    
    ESuComErrors ret = (ESuComErrors) TalkWithSU(password);
    log("SU COM(test): %d\n", ret);
    
    if (ret == SUCOM_ERROR) 
    {
        SetError(SU_ERROR_INTERNAL, "Conversation with su failed(test)\n");
        return false;
    }
    
    if (ret == SUCOM_NULLPASS)
    {
        if (kill(m_iPid, SIGKILL) >= 0) WaitForChild();
        return false;
    }

    if (ret == SUCOM_NOTAUTHORIZED)
    {
        kill(m_iPid, SIGKILL);
        WaitForChild();
        return false;
    }

    WaitForChild();
    return true;
}

bool CLibSU::ExecuteCommand(const char *password, bool removepass)
{
    std::list<std::string> args;

    args.push_back(m_szUser);
    args.push_back("-c");
    args.push_back(std::string("printf \"") + TermStr + "\"; " + m_szCommand); // Insert our magic terminate indicator to command
    args.push_back("-");

    std::string command;
    if (FileExists("/usr/bin/su")) command = "/usr/bin/su";
    else if (FileExists("/bin/su")) command = "/bin/su";

    if (command.empty())
    {
        SetError(SU_ERROR_SUNOTFOUND, "Couldn't find su\n");
        return false;
    }
    
    if (Exec(command, args) < 0)
    {
	return false;
    }
    
    ESuComErrors ret = (ESuComErrors) TalkWithSU(password);
    log("SU COM: %d\n", ret);
    
    if (ret == SUCOM_ERROR) 
    {
        SetError(SU_ERROR_INTERNAL, "Conversation with su failed\n");
        return false;
    }
    
    if (ret == SUCOM_NULLPASS)
    {
        if (kill(m_iPid, SIGKILL) >= 0) WaitForChild();
        return false;
    }

    if (removepass && password)
    {
        char *ptr = const_cast<char *>(password);
        for (unsigned i=0; i<strlen(password); i++)
            ptr[i] = '\000';
    }

    if (ret == SUCOM_NOTAUTHORIZED)
    {
        kill(m_iPid, SIGKILL);
        WaitForChild();
        return false;
    }

    if (WaitForChild() == 127)
    {
        SetError(SU_ERROR_EXECUTE, "Couldn't execute \'%s\'\n", m_szCommand.c_str());
        return false;
    }
    
    return true;
}
