/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

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

#ifndef UTILS_H
#define UTILS_H

#include <dirent.h>
#include <poll.h>
#include <sys/utsname.h>
#include <sys/time.h>

extern std::list<char *> StringList; // List of all strings created by CreateText

void PrintIntro(void);
char *CreateText(const char *s, ...);
inline char *MakeCString(const std::string &s) { return CreateText(s.c_str()); };
char *CreateTmpText(const char *s, ...);
void FreeStrings(void);
bool FileExists(const char *file);
inline bool FileExists(const std::string &file) { return FileExists(file.c_str()); };
bool WriteAccess(const char *file);
inline bool WriteAccess(const std::string &file) { return WriteAccess(file.c_str()); };
bool ReadAccess(const char *file);
inline bool ReadAccess(const std::string &file) { return ReadAccess(file.c_str()); };
bool IsDir(const char *file);
void *guaranteed_memset(void *v,int c,size_t n);
void CleanPasswdString(char *str);
std::string &EatWhite(std::string &str, bool skipnewlines=false);
void EscapeControls(std::string &text);
void GetTextFromBlock(std::ifstream &file, std::string &text);
std::string GetFirstValidDir(const std::string &dir);
std::string GetMD5(const std::string &file);
mode_t StrToMode(const char *str);
std::string GetTranslation(const std::string &s);
const char *GetTranslation(const char *s);
inline char *MakeTranslation(const std::string &s) { return MakeCString(GetTranslation(s)); }
inline char *MakeTranslation(const char *s) { return CreateText(GetTranslation(s)); }
void ConvertTabs(std::string &text);
char *StrDup(const char *str);
void MakeAbsolute(std::string &dir);
std::string GetCWD(void);
void CHDir(const char *dir);
inline void CHDir(const std::string &dir) { CHDir(dir.c_str()); };
void MKDir(const char *dir, int mode);
inline void MKDir(const std::string &dir, int mode) { MKDir(dir.c_str(), mode); };
bool MKDirNeedsRoot(std::string dir);
void MKDirRec(std::string dir);
void MKDirRecRoot(std::string dir, LIBSU::CLibSU &suhandler, const char *passwd);
void UName(struct utsname &u);
int Open(const char *f, int flags);
void GetScaledImageSize(int curw, int curh, int maxw, int maxh, int &outw, int &outh);
std::string LegalNrTokens(bool real, const std::string &curstr, TSTLStrSize pos);
long GetTime(void);
void ConvertExToLuaError(void);
int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

template <typename To, typename From> To SafeConvert(From from)
{
    bool fromsigned = std::numeric_limits<From>::is_signed;
    bool tosigned = std::numeric_limits<To>::is_signed;
    
    if (fromsigned && tosigned)
    {
        if (from > std::numeric_limits<To>::max())
            throw Exceptions::CExOverflow("Detected a overflow with type conversion");
        else if (from < std::numeric_limits<To>::min())
            throw Exceptions::CExOverflow("Detected a underflow with type conversion");
    }
    else if (fromsigned && !tosigned)
    {
        if (from < 0)
            throw Exceptions::CExOverflow("Detected a underflow with type conversion");
        else if (from > std::numeric_limits<To>::max())
            throw Exceptions::CExOverflow("Detected a overflow with type conversion");
    }
    else if (!fromsigned)
    {
        if (from > std::numeric_limits<To>::max())
            throw Exceptions::CExOverflow("Detected a overflow with type conversion");
    }
    
    return static_cast<To>(from);
}

class CDirIter
{
    DIR *m_pDir;
    dirent *m_pEntry;
    
    dirent *GetNextDirEnt(void);
    
public:
    CDirIter(const std::string &dir);
    ~CDirIter(void) { if (m_pDir) closedir(m_pDir); m_pEntry = NULL; };

    CDirIter &operator ++(void);
    CDirIter &operator ++(int) { return this->operator++(); };
    operator bool(void) { return !End(); };
    dirent *operator ->(void);

    bool End(void) { return (m_pEntry == NULL); };
};

class CPipedCMD
{
    std::string m_szCommand;

    int m_iPipeFD[2];
    pid_t m_ChildPID;
    pollfd m_PollData;
    bool m_bChEOF;
    
public:
    CPipedCMD(const char *cmd);
    ~CPipedCMD(void) { Abort(false); };
    
    bool HasData(void);
    bool EndOfFile(void) { return ((m_ChildPID == 0) || m_bChEOF); };
    int GetCh(void);
    int Close(bool canthrow=true); // As pclose: waits for child to exit
    void Abort(bool canthrow=true);
    
    operator bool(void) { return !EndOfFile(); };
};

template <typename C, typename D = C> class CPointerWrapper
{
    C *m_pData;

    typedef void (*delfunc)(D *);
    delfunc m_DelFunc;

    CPointerWrapper(const CPointerWrapper &) {}

public:
    CPointerWrapper(C *d, delfunc f=NULL) : m_pData(d), m_DelFunc(f) { }
    ~CPointerWrapper(void)
    {
        if (m_pData)
        {
            if (m_DelFunc)
                m_DelFunc(m_pData);
            else
                delete m_pData;
        }
    }
    
    C* operator ->(void) { return m_pData; }
    operator C*(void) { return m_pData; }
};

#endif
