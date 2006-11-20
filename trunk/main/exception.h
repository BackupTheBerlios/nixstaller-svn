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

#ifndef EXCEPTION_H
#define EXCEPTION_H
        
#include <exception>

namespace Exceptions {

class CException: public std::exception
{
    const char *m_szMessage;
    
protected:
    const char *FormatText(const char *str, ...)
    {
        static char buffer[512];
        static va_list v;
    
        va_start(v, str);
        vsnprintf(buffer, sizeof(buffer), str, v);
        va_end(v);
        
        return buffer;
    }
    const char *Message(void) { return m_szMessage; };
    
public:
    CException(const char *msg=NULL) : m_szMessage(msg) { };
    virtual const char *what(void) throw() { return m_szMessage; };
};

// Class for grouping IO exceptions
class CExIO: public CException
{
protected:
    CExIO(const char *msg=NULL) : CException(msg) { };
};

class CExOpenDir: public CExIO
{
    int m_iError; // Error code given by errno
    const char *m_szDir;
    
public:
    CExOpenDir(int err, const char *dir) : m_iError(err), m_szDir(dir) { };
    virtual const char *what(void) throw() { return FormatText("Could not open directory %s: %s", m_szDir, strerror(m_iError)); };
};

class CExReadDir: public CExIO
{
    int m_iError; // Error code given by errno
    const char *m_szDir;
    
public:
    CExReadDir(int err, const char *dir) : m_iError(err), m_szDir(dir) { };
    virtual const char *what(void) throw() { return FormatText("Could not read directory %s: %s", m_szDir, strerror(m_iError)); };
};

class CExCurDir: public CExIO
{
    int m_iError; // Error code given by errno
    
public:
    CExCurDir(int err) : m_iError(err) { };
    virtual const char *what(void) throw() { return FormatText("Could not read current directory: %s", strerror(m_iError)); };
};


class CExOverflow: public CException
{
public:
    CExOverflow(const char *msg) : CException(msg) { };
    virtual const char *what(void) throw() { return FormatText("Overflow detected: %s", Message()); };
};

class CExLua: public CException
{
public:
    CExLua(const char *msg) : CException(msg) { };
    virtual const char *what(void) throw() { return FormatText("Lua error detected: %s", Message()); };
};

class CExSU: public CException
{
public:
    CExSU(const char *msg) : CException(msg) { };
    virtual const char *what(void) throw() { return FormatText("Error detected during using su: %s", Message()); };
};

class CExCommand: public CException
{
    const char *m_szCommand;
    
public:
    CExCommand(const char *cmd) : m_szCommand(cmd) { };
    virtual const char *what(void) throw() { return FormatText("Could not execute command: %s", m_szCommand); };
};

class CExNullEntry: public CException
{
public:
    CExNullEntry(void) : CException("Tried to access NULL entry") { };
};

class CExOpenPipe: public CExIO
{
    int m_iError;
    
public:
    CExOpenPipe(int err) : m_iError(err) { };
    virtual const char *what(void) throw() { return FormatText("Could not open pipe: %s", strerror(m_iError)); };
};


class CExPoll: public CException
{
    int m_iError; // Error code given by errno
    
public:
    CExPoll(int err) : m_iError(err) { };
    virtual const char *what(void) throw() { return FormatText("Poll returned a error: %s", strerror(m_iError)); };
};

class CExClosePipe: public CExIO
{
    int m_iError; // Error code given by errno

public:
    CExClosePipe(int err) : m_iError(err) { };
    virtual const char *what(void) throw() { return FormatText("Could not close pipe: %s", strerror(m_iError)); };
};


}


#endif
