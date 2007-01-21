/*
        Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

        This file is part of Nixstaller.

        Nixstaller is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        Nixstaller is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with Nixstaller; if not, write to the Free Software
        Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

        Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
        conditions of the GNU General Public License cover the whole combination.

        In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
        software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
        DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
        such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
        you include the source code of that other code when and as the GNU GPL requires distribution of source code.

        Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
        versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
        version without this exception; this exception also makes it possible to release a modified version which carries forward
        this exception.
*/

#ifndef UTILS_H
#define UTILS_H

#include <dirent.h>
#include <poll.h>
#include <sys/utsname.h>
#include <sys/time.h>

extern std::list<char *> StringList; // List of all strings created by CreateText

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
void GetTextFromBlock(std::ifstream &file, std::string &text);
std::string GetFirstValidDir(const std::string &dir);
std::string GetMD5(const std::string &file);
mode_t StrToMode(const char *str);
std::string GetTranslation(const std::string &s);
char *GetTranslation(char *s);
inline char *GetTranslation(const char *s) { return GetTranslation(const_cast<char *>(s)); };
inline char *MakeTranslation(const std::string &s) { return GetTranslation(MakeCString(s)); }
char *StrDup(const char *str);
std::string GetCWD(void);
void CHDir(const char *dir);
inline void CHDir(const std::string &dir) { CHDir(dir.c_str()); };
void MKDir(const char *dir, int mode);
inline void MKDir(const std::string &dir, int mode) { MKDir(dir.c_str(), mode); };
void UName(struct utsname &u);

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
    FILE *m_pPipe;
    int m_iPipeFD;
    pollfd m_PollData;
    bool m_bChEOF;
    
    void InitPoll(void);
    
public:
    CPipedCMD(const char *cmd, const char *mode);
    ~CPipedCMD(void) { Close(false); };

    int GetCh(void);
    bool HasData(void);
    bool EndOfFile(void) { return (!m_pPipe || feof(m_pPipe) || m_bChEOF); };
    void Close(bool canthrow = true); // canthrow should be false when called from the destructor

    operator bool(void) { return !EndOfFile(); };
};

#endif
