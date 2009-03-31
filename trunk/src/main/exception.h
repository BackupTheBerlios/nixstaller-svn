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

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdio.h>
#include <stdarg.h>
#include <exception>

// Defined by frontend / Nixstbuild
extern const char *GetExTranslation(const char *s);

/* NOTES
    * CException is used for the base class of every exception
    * Base classes from CException should use virtual inheritance(According to boost catch()
      may have trouble with finding the right type otherwise)
    * No exception specifiers are used since they are useless; they add extra overhead and react in a dumb way when a 'wrong' exception is found.
    * The exception type that is caught MUST have a what() function, otherwise the one from std::exception is used(even when the thrown class has its own)
      This function may be pure virtual.
    * All registrated lua function who throw CException type exceptions are catched by DoFunctionCall and transformed to a luaL_error call. This because lua
      will catch every exception by its own and call lua_error afterwards, without any message.
*/
        
namespace Exceptions {

class CException: public std::exception
{
    char m_szBuffer[1024];
    
protected:
    const char *FormatText(const char *str, ...)
    {
        va_list v;
    
        va_start(v, str);
        vsnprintf(m_szBuffer, sizeof(m_szBuffer)-1, str, v);
        m_szBuffer[sizeof(m_szBuffer)-1] = 0;
        va_end(v);
        
        return m_szBuffer;
    }

    void StoreString(const char *str, char *buf, size_t sz)
    {
        strncpy(buf, str, sz-1);
        buf[sz-1] = 0;
    }
    
public:
    CException(void) { /*assert(false);*/ };
    virtual const char *what(void) throw() = 0;
};

// Exception base class for errno based handling(C functions)
class CExErrno: virtual public CException
{
    int m_iError; // errno code
    
protected:
    CExErrno(int err) : m_iError(err) { };
    const char *Error(void) { return strerror(m_iError); };
};

// Exception base class which just returns a given message
class CExMessage: virtual public CException
{
    char m_szMessage[1024];
    
protected:
    CExMessage(const char *msg) { StoreString(msg, m_szMessage, sizeof(m_szMessage)); }
    const char *Message(void) { return m_szMessage; };
    virtual const char *what(void) throw() { return Message(); };
};

// Generic exception, incase specific exception info was lost (eg lua error)
class CExGeneric: public CExMessage
{
public:
    CExGeneric(const char *msg) : CExMessage(msg) { }
};

class CExUsage: public CExMessage
{
public:
    CExUsage(const char *msg) : CExMessage(msg) { }
    virtual const char *what(void) throw() { return FormatText("Wrong usage: %s", Message()); }
};

// Class for grouping IO exceptions
class CExIO: virtual public CException
{
};

class CExOpen: public CExErrno, public CExIO
{
    char m_szFile[1024];
    
public:
    CExOpen(int err, const char *file) : CExErrno(err) { StoreString(file, m_szFile, sizeof(m_szFile)); }
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not open file %s: %s"), m_szFile, Error()); };
};

class CExOpenDir: public CExErrno, public CExIO
{
    char m_szDir[1024];
    
public:
    CExOpenDir(int err, const char *dir) : CExErrno(err) { StoreString(dir, m_szDir, sizeof(m_szDir)); };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not open directory %s: %s"), m_szDir, Error()); };
};

class CExReadDir: public CExIO, public CExErrno
{
    char m_szDir[1024];
    
public:
    CExReadDir(int err, const char *dir) : CExErrno(err) { StoreString(dir, m_szDir, sizeof(m_szDir)); };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not read directory %s: %s"), m_szDir, Error()); };
};

class CExCurDir: public CExErrno, public CExIO
{
public:
    CExCurDir(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not read current directory: %s"), Error()); };
};


class CExOverflow: public CExMessage
{
public:
    CExOverflow(const char *msg) : CExMessage(msg) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Overflow detected: %s"), Message()); };
};

class CExLua: public CExMessage
{
public:
    CExLua(const char *msg) : CExMessage(msg) { };
    virtual const char *what(void) throw() { return FormatText(GetExTranslation("Lua error detected: %s"), Message()); };
};

class CExLuaAbort: public CExMessage
{
public:
    CExLuaAbort(const char *msg) : CExMessage(GetExTranslation(msg)) { };
};

class CExSU: public CExMessage
{
public:
    CExSU(const char *msg) : CExMessage(msg) { };
};

class CExCommand: public CException
{
    char m_szCommand[512];
    
public:
    CExCommand(const char *cmd) { StoreString(cmd, m_szCommand, sizeof(m_szCommand)); };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not execute command: %s"), m_szCommand); };
};

class CExNullEntry: public CExMessage
{
public:
    CExNullEntry(void) : CExMessage("Tried to access NULL entry") { };
};

class CExOpenPipe: public CExErrno, public CExIO
{
public:
    CExOpenPipe(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not open pipe: %s"), Error()); };
};

class CExReadPipe: public CExErrno
{
public:
    CExReadPipe(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not read pipe: %s"), Error()); };
};

class CExClosePipe: public CExErrno, public CExIO
{
public:
    CExClosePipe(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not close pipe: %s"), Error()); };
};

class CExFork: public CExErrno
{
public:
    CExFork(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not fork child: %s"), Error()); };
};

class CExCloseFD: public CExErrno
{
public:
    CExCloseFD(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not close file descriptor: %s"), Error()); };
};

class CExPoll: public CExErrno
{
public:
    CExPoll(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("poll returned an error: %s"), Error()); };
};

class CExSelect: public CExErrno
{
public:
    CExSelect(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("select returned an error: %s"), Error()); };
};

class CExWaitPID: public CExErrno
{
public:
    CExWaitPID(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Encountered an error while waiting on child process: %s"), Error()); };
};

class CExUName: public CExErrno
{
public:
    CExUName(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("uname returned an error: %s"), Error()); };
};

class CExMKDir: public CExErrno, public CExIO
{
public:
    CExMKDir(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText(GetExTranslation("Could not create directory: %s"), Error()); };
};

// Thrown when user wants to quit
class CExUser: public CException
{
    virtual const char *what(void) throw() { return ""; };
};

class CExElf: public CExMessage
{
public:
    CExElf(const char *msg) : CExMessage(msg) { }
    virtual const char *what(void) throw() { return FormatText(GetExTranslation("Elf class error detected: %s"), Message()); };
};


}


#endif
