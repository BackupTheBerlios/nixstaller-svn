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

#include <new>
#include <locale.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fstream>
#include <assert.h>
#include <errno.h>
        
#include "libmd5/md5.h"
#include "utf8.h"
#include "main.h"
#include "lua/lua.h"

std::list<char *> StringList;
extern std::map<std::string, char *> Translations;

void PrintIntro()
{
    std::ifstream file("start");
    char c;
    
    while (file && file.get(c))
        printf("%c", c);
}

char *CreateText(const char *s, ...)
{
    char *txt;
    va_list v;
    
    va_start(v, s);
    vasprintf(&txt, s, v);
    va_end(v);
    
    // Check if string was already created
    if (!StringList.empty())
    {
        for (std::list<char *>::iterator it = StringList.begin(); it != StringList.end(); it++)
        {
            if (!strcmp(*it, txt))
                return *it;
        }
    }
    
    StringList.push_front(txt);
    return txt;
}

// As CreateText, but caller has to free string manually
char *CreateTmpText(const char *s, ...)
{
    char *txt;
    va_list v;
    
    va_start(v, s);
    vasprintf(&txt, s, v);
    va_end(v);
    
    return txt;
}

void FreeStrings()
{
    debugline("freeing %d strings....\n", StringList.size());
    while(!StringList.empty())
    {
        debugline("STRING: %s\n", StringList.back());
        free(StringList.back());
        StringList.pop_back();
    }
}

void FreeTranslations(void)
{
    if (!Translations.empty())
    {
        std::map<std::string, char *>::iterator p = Translations.begin();
        for(; p!=Translations.end(); p++)
            delete [] (*p).second;
        Translations.clear();
    }
}

bool FileExists(const char *file)
{
    struct stat st;
    return (lstat(file, &st) == 0);
}

bool WriteAccess(const char *file)
{
    struct stat st;
    return ((lstat(file, &st) == 0) && (access(file, W_OK) == 0));
}

bool ReadAccess(const char *file)
{
    struct stat st;
    
    if (lstat(file, &st) != 0)
        return false;
    
    int check = (S_ISDIR(st.st_mode)) ? (R_OK | X_OK) : (R_OK);
    return (access(file, check) == 0);
}

bool IsDir(const char *file)
{
    struct stat st;
    
    if (lstat(file, &st) != 0)
        return false;
    
    return (S_ISDIR(st.st_mode));
}

// Incase dir does not exist, it will search for the first valid top directory
std::string GetFirstValidDir(const std::string &dir)
{
    if ((dir[0] == '/') && (dir.length() == 1))
        return dir; // Root dir given

    std::string subdir = dir;
    
    if (!subdir.compare(0, 2, "~/"))
    {
        const char *env = getenv("HOME");
        if (env)
            subdir.replace(0, 1, env);
    }
    
    MakeAbsolute(subdir);
    
    if (ReadAccess(subdir))
        return subdir;
    
    // Remove trailing /
    if (subdir[subdir.length()-1] == '/')
        subdir.erase(subdir.length()-1, 1);
    
    TSTLStrSize pos;
    do
    {
        pos = subdir.rfind('/');
        assert(pos != std::string::npos);
        
        if (pos == std::string::npos)
        {
            // Shouldn't happen
            return subdir;
        }
        else if (pos == 0) // Reached the root dir('/')
            return "/";
        
        subdir.erase(pos);
    }
    while (!ReadAccess(subdir));
    
    return subdir;
}

// From http://tldp.org/HOWTO/Secure-Programs-HOWTO/protect-secrets.html
void *guaranteed_memset(void *v, int c, size_t n)
{
    volatile char *p = (volatile char *)v;
    while (n)
    {
        n--;
        *(p++) = c;
    }
    return v;
}

// Used for cleaning password strings
void CleanPasswdString(char *str)
{
    if (str)
    {
        guaranteed_memset(str, 0, strlen(str));
        free(str);
    }
}

std::string &EatWhite(std::string &str, bool skipnewlines)
{
    const char *filter = (skipnewlines) ? " \t" : " \t\r\n";
    TSTLStrSize pos = str.find_first_not_of(filter);

    if (pos != std::string::npos)
        str.erase(0, pos);
    
    pos = str.find_last_not_of(filter);

    if (pos != std::string::npos)
        str.erase(pos+1);
    
    return str;
}

void EscapeControls(std::string &text)
{
    TSTLStrSize start = 0;
    while (start < text.length())
    {
        start = text.find_first_of("%", start);
        
        if (start != std::string::npos)
        {
            text.replace(start, 1, "%%");
            start += 2;
            continue;
        }
        else
            break;

        start++;
    }
}

// Used by config file parsing, gets string between a text block
void GetTextFromBlock(std::ifstream &file, std::string &text)
{
    std::string tmp;
    text.erase(0, 1); // Remove [
    EatWhite(text);
            
    while (file)
    {
        std::getline(file, tmp);
        text += '\n' + EatWhite(tmp);
        if (text[text.length()-1] == ']')
        {
            // Don't use "\]" as exit point(this way we can use a ] in a text block)
            if ((text.length() > 1) && (text[text.length()-2] == '\\'))
                text.erase(text.length()-2, 1);
            else
                break;
        }
    }
    
    if (text[text.length()-1] == ']')
    {
        text.erase(text.length()-1, 1); // Remove ]
        EatWhite(text);
    }
}

std::string GetMD5(const std::string &file)
{
    FILE *fp = fopen(file.c_str(), "rb");
    
    if (!fp)
        return "0";

    unsigned bytes = 0;
    char buf[4096];
    md5_state_t state;
    md5_byte_t digest[16];
    char hex_output[16*2 + 1];
    
    md5_init(&state);
    
    while((bytes = fread(buf, 1, sizeof(buf), fp)))
    {
        md5_append(&state, (const md5_byte_t *)buf, bytes);
        char txt[4097];
        strncpy(txt, buf, 4096);
        txt[4096] = 0;
    }

    md5_finish(&state, digest);

    for (int di = 0; di < 16; ++di)
        sprintf(hex_output + di * 2, "%02x", digest[di]);
    
    fclose(fp);
    return hex_output;
}
                                           
mode_t StrToMode(const char *str)
{
    // Code from GNU's coreutils
    unsigned int octal_value = 0;

    do
    {
        octal_value = 8 * octal_value + *str++ - '0';
        if (07777 < octal_value)
            throw Exceptions::CExOverflow("Too high octal number given for file permission mode");
    }
    while ('0' <= *str && *str < '8');

    return octal_value;
}

std::string GetTranslation(const std::string &s)
{
    std::map<std::string, char *>::iterator p = Translations.find(s);
    if (p != Translations.end())
        return (*p).second;
    
#ifndef RELEASE
    if (!Translations.empty())
    {
        std::string esc = s;
        EscapeControls(esc);
        debugline("WARNING: No translation for %s\n", esc.c_str());
    }
#endif
    
    return s;
}

const char *GetTranslation(const char *s)
{
    std::map<std::string, char *>::iterator p = Translations.find(s);
    if (p != Translations.end())
        return (*p).second;
    
    // No translation found
    
#ifndef RELEASE
    if (!Translations.empty())
    {
        std::string esc = s;
        EscapeControls(esc);
        debugline("WARNING: No translation for %s\n", esc.c_str());
    }
#endif
    
    return s;
}

void ConvertTabs(std::string &text)
{
    TSTLStrSize length = text.length(), start = 0, startline = 0;
    while (start < length)
    {
        start = text.find_first_of("\t\n", start);
        
        if (start != std::string::npos)
        {
            if (text[start] == '\n')
                startline = 0;
            else
            {
                int rest = (startline) ? (8 % startline) : 0;
                text.replace(start, 1, 8-rest, ' ');
            }
        }
        else
            break;

        start++;
    }
}

char *StrDup(const char *str)
{
    char *ret = strdup(str);
    
    if (!ret)
        throw std::bad_alloc();
    
    return ret;
}

void MakeAbsolute(std::string &dir)
{
    if (dir[0] != '/')
        dir = GetCWD() + "/" + dir;
}

std::string GetCWD()
{
    static char buffer[1024];
    
    if (!getcwd(buffer, sizeof(buffer)))
        throw Exceptions::CExCurDir(errno);
    
    return buffer;
}

void CHDir(const char *dir)
{
    if (chdir(dir))
        throw Exceptions::CExReadDir(errno, dir);
}

void MKDir(const char *dir, int mode)
{
    if (mkdir(dir, mode) < 0)
        throw Exceptions::CExMKDir(errno);
}

bool MKDirNeedsRoot(std::string dir)
{
    MakeAbsolute(dir);
    
    TSTLStrSize start = 1; // Skip first root path
    TSTLStrSize end = 0;
    bool needroot = false;
    
    // Get first directory to create
    do
    {
        end = dir.find("/", start);
        std::string subdir = dir.substr(0, end);
            
        if (FileExists(subdir))
            needroot = !WriteAccess(subdir);
        else
            break;
        
        start = end + 1;
    }
    while (end != std::string::npos);
    
    return needroot;
}

void MKDirRec(std::string dir)
{
    MakeAbsolute(dir);

    TSTLStrSize start = 1; // Skip first root path
    TSTLStrSize end = 0;
    bool create = false; // Start creating directories?
    
    do
    {
        end = dir.find("/", start);
        std::string subdir = dir.substr(0, end);
            
        if (!create)
            create = !FileExists(subdir);
        
        if (create)
            MKDir(subdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH));
        
        start = end + 1;
    }
    while (end != std::string::npos);
}

void MKDirRecRoot(std::string dir, LIBSU::CLibSU &suhandler, const char *passwd)
{
    MakeAbsolute(dir);
    
    suhandler.SetCommand("mkdir -p " + dir);
    
    if (!suhandler.ExecuteCommand(passwd))
        throw Exceptions::CExRootMKDir(dir.c_str());
}

void UName(struct utsname &u)
{
    if (uname(&u) == -1)
        throw Exceptions::CExUName(errno);
}

void Pipe(int fd[2])
{
    if (pipe(fd) == -1)
        throw Exceptions::CExOpenPipe(errno);
}

pid_t Fork()
{
    pid_t ret = fork();
    
    if (ret == -1)
        throw Exceptions::CExFork(errno);
    
    return ret;
}

int Open(const char *f, int flags)
{
    int ret = open(f, flags);
    if (ret == -1)
        throw Exceptions::CExOpen(errno, f);
    return ret;
}

void Close(int fd)
{
    if (close(fd) == -1)
        throw Exceptions::CExCloseFD(errno);
}

pid_t WaitPID(pid_t pid, int *status, int options)
{
    pid_t ret = waitpid(pid, status, options);
    
    if (ret == -1)
        throw Exceptions::CExWaitPID(errno);
    
    return ret;
}

void GetScaledImageSize(int curw, int curh, int maxw, int maxh, int &outw, int &outh)
{
    float wfact, hfact;

    // Check which scale factor is the biggest and use that one. This makes sure that the image will fit.
    wfact = (static_cast<float>(curw) / static_cast<float>(maxw));
    hfact = (static_cast<float>(curh) / static_cast<float>(maxh));

    if (wfact >= hfact)
    {
        outw = maxw;
        outh = static_cast<int>(static_cast<float>(curh) / wfact);
    }
    else
    {
        outw = static_cast<int>(static_cast<float>(curw) / hfact);
        outh = maxh;
    }
}

// Returns a string that contains valid characters for a 'numeric' string. Used for user input.
std::string LegalNrTokens(bool real, const std::string &curstr, TSTLStrSize pos)
{
    lconv *lc = localeconv();
    std::string legal = "1234567890";
    
    assert(lc != NULL);
                    
    std::string decpoint = std::string(lc->decimal_point);
    std::string plusminsign = std::string(lc->positive_sign) + std::string(lc->negative_sign);
                    
    if (real && (curstr.find_first_of(decpoint) == std::string::npos))
        legal += decpoint;
                    
    if ((pos == 0) && (curstr.find_first_of(plusminsign) == std::string::npos))
        legal += plusminsign;
    
    return legal;
}

long GetTime(void)
{
    static timeval start, current;
    static bool started = false;
    
    if (!started)
    {
        gettimeofday(&start, NULL);
        started = true;
    }
        
    gettimeofday(&current, NULL);
    return ((current.tv_sec-start.tv_sec) * 1000 ) + ((current.tv_usec-start.tv_usec) / 1000);
}

void ConvertExToLuaError()
{
    try
    {
        throw;
    }
    catch(Exceptions::CExUser &e)
    {
        // HACK: ConvertLuaErrorToEx() uses this to identify user exception.
        lua_pushstring(NLua::LuaState, "CExUser");
        lua_error(NLua::LuaState);
    }
    catch(Exceptions::CException &e)
    {
        // HACK: ConvertLuaErrorToEx() uses this to seperate regular exceptions from lua errors.
        lua_pushfstring(NLua::LuaState, "CException%s", e.what());
        lua_error(NLua::LuaState);
    }
}

void ConvertLuaErrorToEx()
{
    const char *errmsg = lua_tostring(NLua::LuaState, -1);
    const size_t exlen = strlen("CException");
    
    if (!errmsg)
        errmsg = "Unknown error!";
    else if (!strcmp(errmsg, "CExUser"))
        throw Exceptions::CExUser();
    else if (!strncmp(errmsg, "CException", exlen))
        throw Exceptions::CExGeneric(&errmsg[exlen]);
        
    throw Exceptions::CExLua(errmsg);
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int ret = select(nfds, readfds, writefds, exceptfds, timeout);
    if (ret == -1)
        throw Exceptions::CExSelect(errno);
    return ret;
}

TSTLStrSize MBWidth(std::string str)
{
    if (str.empty())
        return 0;

    // Remove newlines, as wcswidth can't really handle them
    TSTLStrSize start = 0;
    while (start < str.length())
    {
        start = str.find_first_of("\n", start);
        
        if (start != std::string::npos)
        {
            str.erase(start, 1);
            // keep start the same as 1 char was just removed
            continue;
        }
        else
            break;

        start++;
    }
    
    TSTLStrSize ret = 0;
    std::wstring utf16;
    std::string::iterator end = utf8::find_invalid(str.begin(), str.end());
    assert(end == str.end());
    
    // HACK: Just append length of invalid part
    if (end != str.end())
        ret = std::distance(end, str.end());
    
    utf8::utf8to16(str.begin(), end, std::back_inserter(utf16));
    ret += wcswidth(utf16.c_str(), utf16.length());
    
    return ret;
}

TSTLStrSize GetMBLenFromW(const std::string &str, size_t width)
{
    if (str.empty())
        return 0;

    utf8::iterator<std::string::const_iterator> it(str.begin(), str.begin(), str.end());
    for (; it.base() != str.end(); it++)
    {
        TSTLStrSize w = MBWidth(std::string(str.begin(), it.base()));
        if (w == width)
            break;
        else if (w > width) // This may happen with multi-column chars
        {
            if (it.base() != str.begin())
                it--;
            break;
        }
    }
    
    return std::distance(str.begin(), it.base());
}

TSTLStrSize GetMBWidthFromC(const std::string &str, std::string::const_iterator cur, int n)
{
    if (str.empty())
        return 0;
    
    std::string::const_iterator it = cur;
    if (n < 0)
    {
        while ((n < 0) && (it != str.begin()))
        {
            utf8::prior(it, str.begin());
            n++;
        }
        return MBWidth(std::string(it, cur));
    }
    else
    {
        while ((n > 0) && (it != str.end()))
        {
            utf8::next(it, str.end());
            n--;
        }
        return MBWidth(std::string(cur, it));
    }
}

TSTLStrSize GetFitfromW(const std::string &str, std::string::const_iterator cur, TSTLStrSize width, bool roundup)
{
    std::string::const_iterator it = cur;
    TSTLStrSize prev = 0;
    while (it != str.end())
    {
        utf8::next(it, str.end());
        TSTLStrSize w = MBWidth(std::string(cur, it));
        if (w == width)
            return w;
        else if (w > width)
            return (roundup) ? w : prev;
        prev = w;
    }
    return width;
}

#if 0
size_t MBWidth(std::string str)
{
    // Remove newlines, as wcswidth can't really handle them
    TSTLStrSize start = 0;
    while (start < str.length())
    {
        start = str.find_first_of("\n", start);
        
        if (start != std::string::npos)
        {
            str.erase(start, 1);
            // keep start the same as 1 char was just removed
            continue;
        }
        else
            break;

        start++;
    }
    
    wchar_t *wc = ToWC(str.c_str());
    if (wc)
    {
        size_t ret = wcswidth(wc, wcslen(wc));
        debugline("MBWidth: %s: %u, %u\n", str.c_str(), wcslen(wc), wcswidth(wc, wcslen(wc)));
        delete [] wc;
        return ret;
    }
    else
        return str.length();
}
#endif

// -------------------------------------
// Directory iterator
// -------------------------------------

CDirIter::CDirIter(const std::string &dir) : m_pEntry(NULL)
{
    m_pDir = opendir(dir.c_str());
    
    if (m_pDir)
        m_pEntry = GetNextDirEnt();
    else
        throw Exceptions::CExOpenDir(errno, dir.c_str());
}

dirent *CDirIter::GetNextDirEnt()
{
    dirent *entry = readdir(m_pDir);
    
    while (entry && !strcmp(entry->d_name, "."))
        entry = readdir(m_pDir);
    
    return entry;
}

CDirIter &CDirIter::operator ++()
{
    if (m_pEntry)
        m_pEntry = GetNextDirEnt();
    else
        throw Exceptions::CExOverflow("Tried to increment directory iter after last directory was reached");
    
    return *this;
}

dirent *CDirIter::operator ->()
{
    if (End())
        throw Exceptions::CExNullEntry();
    
    return m_pEntry;
}

// -------------------------------------
// Frontend class for popen
// -------------------------------------

CPipedCMD::CPipedCMD(const char *cmd) : m_szCommand(cmd), m_ChildPID(0), m_bChEOF(false)
{
    Pipe(m_iPipeFD);
    
    m_ChildPID = Fork();
    
    if (m_ChildPID == 0) // Child
    {
        if (setsid() == -1)
            _exit(127);
        
        // Redirect to stdout
        dup2(m_iPipeFD[1], STDOUT_FILENO);
        close(m_iPipeFD[0]);
        close(m_iPipeFD[1]);
        
        execlp("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(127);
    }
    else if (m_ChildPID > 0) // Parent
    {
        ::Close(m_iPipeFD[1]);
        m_PollData.fd = m_iPipeFD[0];
        m_PollData.events = POLLIN | POLLHUP;
    }
}

int CPipedCMD::GetCh()
{
    int ret;
    if (read(m_iPipeFD[0], &ret, 1) == 0)
    {
        ret = EOF;
        m_bChEOF = true;
    }
    
    return ret;
}

bool CPipedCMD::HasData()
{
    if (EndOfFile())
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
    else if (((m_PollData.revents & POLLIN) == POLLIN) || ((m_PollData.revents & POLLHUP) == POLLHUP))
        return true;
    
    return false;
}

int CPipedCMD::Close(bool canthrow)
{
    if (!m_ChildPID)
        return -1;
    
    if (canthrow) 
        ::Close(m_iPipeFD[0]);
    else
        close(m_iPipeFD[0]);
    
    pid_t ret;
    int stat;
    do
    {
        ret = (canthrow) ? WaitPID(m_ChildPID, &stat, 0) : waitpid(m_ChildPID, &stat, 0);
    }
    while ((ret == -1) && (errno == EINTR));
    
    m_ChildPID = 0;
    
    if (canthrow)
    {
        if (!WIFEXITED(stat) || (WEXITSTATUS(stat) == 127))
            throw Exceptions::CExCommand(m_szCommand.c_str());
    }
    
    return WEXITSTATUS(stat);
}

void CPipedCMD::Abort(bool canthrow)
{
    if (m_ChildPID)
    {
        if (kill(-m_ChildPID, SIGTERM) >= 0)
            (canthrow) ? WaitPID(m_ChildPID, NULL, 0) : waitpid(m_ChildPID, NULL, 0);
        Close(canthrow);
    }
}
