#ifndef LSU_LIBSU_H
#define LSU_LIBSU_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <list>
#include <signal.h>

// UNDONE: Check what should be private/protected/public
class CLibSU
{
    enum CheckPidStatus { Error=-1, NotExited=-2, Killed=-3 } ;
    enum SuErrors { error=-1, ok=0, killme=1, notauthorized=2 } ;
    enum Errors { SuNotFound=1, SuNotAllowed, SuIncorrectPassword };
    enum checkMode { NoCheck=0, Install=1, NeedPassword=2 } ;

    int m_iPTYFD, m_iPid;
    std::string m_szPTYName, m_szTTYName, m_szInBuf;
    std::string m_szCommand, m_szUser;
    std::string m_szExit;
    
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
    virtual ~CLibSU(void) { if (m_iPTYFD) close(m_iPTYFD); };
    
    void SetCommand(const char *command) { m_szCommand = command; };
    void SetCommand(const std::string &command) { m_szCommand = command; };
    void SetUser(const char *user) { m_szUser = user; };
    void SetUser(const std::string &user) { m_szUser = user; };
    void SetExitString(const char *exit) { m_szExit = exit; };
    void SetExitString(const std::string &exit) { m_szExit = exit; };

    int ExecuteCommand(const char *password, int check);
};

#endif
