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
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

#include "libmd5/md5.h"
#include "utf8.h"
#include "main.h"
#include "lua/lua.h"

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

bool IsLink(const char *file)
{
    struct stat st;
    
    if (lstat(file, &st) != 0)
        return false;
    
    return (S_ISLNK(st.st_mode));
}

off_t FileSize(const char *file)
{
    struct stat st;
    LStat(file, &st);
    return st.st_size;
}

// In case dir does not exist, it will search for the first valid top directory
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

void StringReplace(std::string &text, const std::string &oldsub,
                   const std::string &newsub)
{
    const TSTLStrSize oldslen = oldsub.length();
    const TSTLStrSize newslen = newsub.length();
    TSTLStrSize start = 0, length = text.length();
    while (start < length)
    {
        start = text.find(oldsub, start);
        if (start != std::string::npos)
        {
            text.replace(start, oldslen, newsub);
            start += newslen;
            length = text.length();
            continue;
        }
        else
            break;
        
        start++;
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

void MKDirRec(std::string dir)
{
    MakeAbsolute(dir);

    TSTLStrSize start = 1; // Skip first root path
    TSTLStrSize end = 0;
    
    do
    {
        end = dir.find("/", start);
        std::string subdir = dir.substr(0, end);

        if (subdir.empty())
            break;       
        
        if (!FileExists(subdir))
            MKDir(subdir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH));
        
        start = end + 1;
    }
    while (end != std::string::npos);
}

void UName(struct utsname &u)
{
    if (uname(&u) == -1)
        throw Exceptions::CExUName(errno);
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

int Read(int fd, void *buf, size_t count)
{
    int ret = read(fd, buf, count);
    if (ret == -1)
        throw Exceptions::CExRead(errno);
    return ret;
}

int Write(int fd, const void *buf, size_t count)
{
    int ret = write(fd, buf, count);
    if (ret == -1)
        throw Exceptions::CExWrite(errno);
    return ret;
}

pid_t WaitPID(pid_t pid, int *status, int options)
{
    pid_t ret = waitpid(pid, status, options);
    
    if ((ret == -1) && (errno != EINTR))
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

TSTLStrSize MBWidth(std::string str)
{
    if (str.empty())
        return 0;
    
    TSTLStrSize ret = 0;
    std::wstring utf16;
    std::string::iterator end = utf8::find_invalid(str.begin(), str.end());
    assert(end == str.end());
    
    // HACK: Just append length of invalid part
    if (end != str.end())
        ret = std::distance(end, str.end());
    
    utf8::utf8to16(str.begin(), end, std::back_inserter(utf16));

    // As wcswidth returns -1 when it finds a non printable char, we just use
    // wcwidth and assume a 0 width for those chars.
    TSTLStrSize n = 0, length = utf16.length();
    for (; n<length; n++)
    {
        int w = wcwidth(utf16[n]);
        if (w > 0)
            ret += static_cast<TSTLStrSize>(w);
    }
    
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

std::string JoinPath(const std::string &path, const std::string &file)
{
    return path + "/" + file;
}

void FStat(int fd, struct stat *buf)
{
    if (fstat(fd, buf) != 0)
        throw Exceptions::CExStat(errno);
}

void LStat(const char *file, struct stat *buf)
{
    if (lstat(file, buf) != 0)
        throw Exceptions::CExStat(errno);
}

void FChMod(int fd, mode_t mode)
{
    if (fchmod(fd, mode) != 0)
        throw Exceptions::CExChMod(errno);
}

void MakeOptDest(const std::string &src, std::string &dest)
{
    TSTLStrSize len = dest.length();
    
    // Strip trailing /'s
    while (len && (dest[len-1] == '/'))
    {
        dest.erase(len-1, 1);
        len--;
    }
    
    if (IsDir(dest))
        dest = JoinPath(dest, BaseName(src));
}

void SymLink(const char *src, const char *dest)
{
    if (symlink(src, dest) == -1)
        throw Exceptions::CExSymLink(errno, dest);
}

void CopyFile(const std::string &src, std::string dest,
              TFileOpCB cb, void *prvdata)
{
    MakeOptDest(src, dest);
    
    if (src == dest)
        return;

    if (IsLink(src))
    {
        if (FileExists(dest) && !IsDir(dest))
            Unlink(dest.c_str());
        SymLink(src, dest);
        return;
    }
    
    CFDWrapper in(Open(src.c_str(), O_RDONLY));

    mode_t flags = O_WRONLY | (FileExists(dest) ? O_TRUNC : O_CREAT);
    CFDWrapper out(Open(dest.c_str(), flags));

    char buffer[1024];
    size_t size;
    while((size = Read(in, buffer, sizeof(buffer))))
    {
        int b = Write(out, buffer, size);
        if (cb)
            cb(b, prvdata);
    }

    // Copy file permissions
    struct stat st;
    FStat(in, &st);

    // Disable umask temporary so we can normally copy permissions
    mode_t mask = umask(0);
    FChMod(out, st.st_mode);
    umask(mask);
}

void Unlink(const char *file)
{
    if (unlink(file) != 0)
        throw Exceptions::CExUnlink(errno, file);
}

void MoveFile(const std::string &src, std::string dest,
              TFileOpCB cb, void *prvdata)
{
    
    MakeOptDest(src, dest);
    
    if (src == dest)
        return;
    
    int ret = rename(src.c_str(), dest.c_str());
    if (ret != -1)
        return; // Hooray, quick and fast move :)
    else if (errno != EXDEV)
        throw Exceptions::CExMove(errno);
    
    // Couldn't rename file because it's on another disk; copy & delete
    
    try
    {
        CopyFile(src, dest, cb, prvdata);
    }
    catch(Exceptions::CExChMod &)
    {
        // Always unlink
        Unlink(src);
        throw;
    }

    Unlink(src);
}

std::string DirName(const std::string &s)
{
    std::string ret = s;
    TSTLStrSize ind = ret.rfind("/");
    
    if (ind != std::string::npos)
        ret.erase(ind);
    else
        ret = ".";
    
    return ret;
}

std::string BaseName(std::string s)
{
    TSTLStrSize ind = s.rfind("/");
    
    if (ind != std::string::npos)
        return s.substr(ind+1);
    
    return s;
}

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

// Don't forget to free() the text...
char *CreateTmpText(const char *s, ...)
{
    char *txt;
    va_list v;
    
    va_start(v, s);
    vasprintf(&txt, s, v);
    va_end(v);
    
    return txt;
}
