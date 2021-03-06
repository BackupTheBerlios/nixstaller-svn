/*
    Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)

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
        
#include <exception>

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
    char m_szBuffer[512];
    
protected:
    const char *FormatText(const char *str, ...)
    {
        va_list v;
    
        va_start(v, str);
        vsnprintf(m_szBuffer, sizeof(m_szBuffer), str, v);
        va_end(v);
        
        return m_szBuffer;
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
    const char *m_szMessage;
    
protected:
    CExMessage(const char *msg) : m_szMessage(msg) { };
    const char *Message(void) { return m_szMessage; };
    virtual const char *what(void) throw() { return Message(); };
};


// Class for grouping IO exceptions
class CExIO: virtual public CException
{
};

class CExOpenDir: public CExErrno, public CExIO
{
    const char *m_szDir;
    
public:
    CExOpenDir(int err, const char *dir) : CExErrno(err), m_szDir(dir) { };
    virtual const char *what(void) throw() { return FormatText("Could not open directory %s: %s", m_szDir, Error()); };
};

class CExReadDir: public CExIO, public CExErrno
{
    const char *m_szDir;
    
public:
    CExReadDir(int err, const char *dir) : CExErrno(err), m_szDir(dir) { };
    virtual const char *what(void) throw() { return FormatText("Could not read directory %s: %s", m_szDir, Error()); };
};

// Thrown when target extraction dir could not be read and cannot be selected by user.
class CExReadExtrDir: public CExIO
{
    const char *m_szDir;
    
public:
    CExReadExtrDir(const char *dir) : m_szDir(dir) { };
    virtual const char *what(void) throw()
    { return FormatText("This installer will install files to the following directory:\n%s\n"
                        "However you don't have read permissions to this directory\n"
                        "Please restart the installer as a user who does or as the root user", m_szDir); };
};

class CExCurDir: public CExErrno, public CExIO
{
public:
    CExCurDir(int err) : CExErrno(err) { };
    virtual const char *what(void) throw() { return FormatText("Could not read current directory: %s", Error()); };
};


class CExOverflow: public CExMessage
{
public:
    CExOverflow(const char *msg) : CExMessage(msg) { };
    virtual const char *what(void) throw() { return FormatText("Overflow detected: %s", Message()); };
};

class CExLua: public CExMessage
{
public:
    CExLua(const char *msg) : CExMessage(msg) { };
    virtual const char *what(void) throw() { return FormatText("Lua error detected: %s", Message()); };
};

class CExSU: public CExMessage
{
public:
    CExSU(const char *msg) : CExMessage(msg) { };
};

class CExCommand: public CException
{
    const char *m_szCommand;
    
public:
    CExCommand(const char *cmd) : m_szCommand(cmd) { };
    virtual const char *what(void) throw() { return FormatText("Could not execute command: %s", m_szCommand); };
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
    virtual const char *what(void) throw() { return FormatText("Could not open pipe: %s", Error()); };
};

class CExReadPipe: public CExErrno
{
public:
    CExReadPipe(int err) : CExErrno(err) { };
    virtual const char *what(void) throw() { return FormatText("Could not read pipe: %s", Error()); };
};

class CExClosePipe: public CExErrno, public CExIO
{
public:
    CExClosePipe(int err) : CExErrno(err) { };
    virtual const char *what(void) throw() { return FormatText("Could not close pipe: %s", Error()); };
};

class CExFork: public CExErrno
{
public:
    CExFork(int err) : CExErrno(err) { };
    virtual const char *what(void) throw() { return FormatText("Could not fork child: %s", Error()); };
};

class CExCloseFD: public CExErrno
{
public:
    CExCloseFD(int err) : CExErrno(err) { };
    virtual const char *what(void) throw() { return FormatText("Could not close file descriptor: %s", Error()); };
};

class CExPoll: public CExErrno
{
public:
    CExPoll(int err) : CExErrno(err) { };
    virtual const char *what(void) throw() { return FormatText("Poll returned an error: %s", Error()); };
};

class CExWaitPID: public CExErrno
{
public:
    CExWaitPID(int err) : CExErrno(err) { };
    virtual const char *what(void) throw()
    { return FormatText("Encountered an error while waiting on child process: %s", Error()); };
};

class CExUName: public CExErrno
{
public:
    CExUName(int err) : CExErrno(err) { };
    virtual const char *what(void) throw() { return FormatText("uname returned an error: %s", Error()); };
};

class CExMKDir: public CExErrno, public CExIO
{
public:
    CExMKDir(int err) : CExErrno(err) { };
    virtual const char *what(void) throw() { return FormatText("Could not create directory: %s", Error()); };
};

class CExFrontend: public CExMessage
{
public:
    CExFrontend(const char *msg) : CExMessage(msg) { };
};

// Thrown when user wants to quit
class CExUser: public CException
{
    virtual const char *what(void) throw() { return ""; };
};

}


#endif
