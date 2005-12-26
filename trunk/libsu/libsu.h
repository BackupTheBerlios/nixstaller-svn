#ifndef LSU_LIBSU_H
#define LSU_LIBSU_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <list>
#include <signal.h>

namespace LIBSU
{

//#define ENABLE_LOGGING /* If set, debug stuff will be logged to log.txt file */

// UNDONE: Check what should be private/protected/public
class CLibSU
{
public:
    enum ESuErrors { SU_ERROR_NONE, SU_ERROR_SUNOTFOUND, SU_ERROR_INCORRECTUSER, SU_ERROR_INCORRECTPASS, SU_ERROR_EXECUTE,
                     SU_ERROR_INTERNAL };

    typedef void (*TThinkFunc)(void *p);
    typedef void (*TOutputFunc)(const char *s, void *p);
    
    CLibSU(bool Disable0Core=false);
    CLibSU(const char *command, const char *user=NULL, const char *path="/bin:/usr/bin", bool Disable0Core=false);
    virtual ~CLibSU(void) { if (m_iPTYFD) close(m_iPTYFD); };
    
    void SetCommand(const char *command) { m_szCommand = command; };
    void SetCommand(const std::string &command) { m_szCommand = command; };
    void SetUser(const char *user) { m_szUser = user; };
    void SetUser(const std::string &user) { m_szUser = user; };
    void SetPath(const char *path) { m_szPath = path; };
    void SetPath(const std::string &path) { m_szPath = path; };
    void SetExitString(const char *exit) { m_szExit = exit; };
    void SetExitString(const std::string &exit) { m_szExit = exit; };
    void SetTerminalOutput(bool t) { m_bTerminal = t; };
    void SetThinkFunc(TThinkFunc f, void *p=NULL) { m_pThinkFunc = f; m_pCustomThinkData = p; };
    void SetOutputFunc(TOutputFunc f, void *p=NULL) { m_pOutputFunc = f; m_pCustomOutputData = p; };
    
    ESuErrors GetError(void) { return m_eError; };
    std::string GetErrorMsg(void) { return m_szErrorMsg; };
    const char *GetErrorMsgC(void) { return m_szErrorMsg.c_str(); };
    
    bool NeedPassword(void);
    bool TestSU(const char *password);
    bool ExecuteCommand(const char *password, bool removepass=false);

private:
    enum EPidStatus { PID_ERROR=-1, PID_NOTEXITED=-2, PID_KILLED=-3 } ;
    enum ESuComErrors { SUCOM_ERROR=-1, SUCOM_OK=0, SUCOM_NULLPASS=1, SUCOM_NOTAUTHORIZED=2 } ;

    int m_iPTYFD, m_iPid;
    bool m_bTerminal;
    std::string m_szPTYName, m_szTTYName, m_szInBuf;
    std::string m_szCommand, m_szUser, m_szPath;
    std::string m_szExit;
    
    ESuErrors m_eError;
    std::string m_szErrorMsg;
    
    TThinkFunc m_pThinkFunc;
    TOutputFunc m_pOutputFunc;
    void *m_pCustomThinkData, *m_pCustomOutputData;
    
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
    
    void SetError(ESuErrors errtype, const char *msg, ...);
    
    int TalkWithSU(const char *password);
};

};

#endif
