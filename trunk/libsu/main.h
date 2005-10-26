// This project is heavily based on 'kdelibsu'

#ifndef LSU_MAIN_H
#define LSU_MAIN_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <string>
#include <list>

#include "config.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE   /* Needed for getpt, ptsname in glibc 2.1.x systems */
#endif

#if defined(__osf__)
#include <pty.h>
#endif

// stdlib.h is meant to declare the prototypes but doesn't :(
#ifndef __THROW
#define __THROW
#endif

#ifdef HAVE_GRANTPT
extern "C" int grantpt(int fd) __THROW;
#endif

#ifdef HAVE_PTSNAME
extern "C" char * ptsname(int fd) __THROW;
#endif

#ifdef HAVE_UNLOCKPT
extern "C" int unlockpt(int fd) __THROW;
#endif

#ifdef HAVE__GETPTY
extern "C" char *_getpty(int *, int, mode_t, int);
#endif

#ifdef HAVE__PTY_H
    #include <pty.h>
#endif

#include <termios.h>

#ifdef HAVE_LIBUTIL_H
    #include <libutil.h>
#elif defined(HAVE_UTIL_H)
    #include <util.h>
#endif

#if defined(__SVR4) && defined(sun)
#include <stropts.h>
#include <sys/stream.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>                // Needed on some systems.
#endif



/*
 * Steven Schultz <sms at to.gd-es.com> tells us :
 * BSD/OS 4.2 doesn't have a prototype for openpty in its system header files
 */
#ifdef __bsdi__
__BEGIN_DECLS
int openpty(int *, int *, char *, struct termios *, struct winsize *);
__END_DECLS
#endif

enum CheckPidStatus { Error=-1, NotExited=-2, Killed=-3 } ;
enum SuErrors { error=-1, ok=0, killme=1, notauthorized=2 } ;
enum Errors { SuNotFound=1, SuNotAllowed, SuIncorrectPassword };
enum checkMode { NoCheck=0, Install=1, NeedPassword=2 } ;

// UNDONE: Check what should be private/protected/public
class CLibSU
{
    int m_iPTYFD, m_iPid;
    std::string m_szPTYName, m_szTTYName, m_szInBuf;
    std::string m_szCommand, m_szUser;
    
    int CreatePT(void);
    int GrantPT(void);
    int UnlockPT(void);
    
    int Exec(const std::string &command, const std::list<std::string> &args);
    int SetupTTY(int fd);
    std::string ReadLine(bool block=true);
    void WriteLine(const std::string &line, bool AddLine=true);
    void UnReadLine(const std::string &line, bool AddLine=true);
    int WaitMS(int fd, int ms);
    int WaitSlave(void);
    int WaitForChild(void);
    int CheckPidExited(pid_t pid);
    bool CheckPid(pid_t pid) { return kill(pid,0) == 0; };
    
    int TalkWithHelper(int check);
    int TalkWithSU(const char *password);
    
public:
    CLibSU(const char *command = NULL, const char *user=NULL) : m_iPTYFD(0), m_iPid(0), m_szCommand(command),
                                                                m_szUser(user) { };
    virtual ~CLibSU(void) { if (m_iPTYFD) close(m_iPTYFD); extern void close_log(void); close_log(); };
    
    void SetCommand(const char *command) { m_szCommand = command; };
    void SetUser(const char *user) { m_szUser = user; };
    
    int ExecuteCommand(const char *password, int check);
};

#endif
