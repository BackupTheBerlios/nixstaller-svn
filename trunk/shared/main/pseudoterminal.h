/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.
    
    This file contains code of the KDE project, module kdesu.
    Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
    
    This file contains code from TEShell.C of the KDE konsole.
    Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>
    
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

#ifndef PSEUDO_TERMINAL_H
#define PSEUDO_TERMINAL_H

#include <unistd.h>
#include <poll.h>
#include <string>

class CPseudoTerminal
{
    std::string m_TTYName;
    int m_iPTYFD;
    pid_t m_ChildPid;
    bool m_bEOF;
    pollfd m_PollData;
    bool m_bPidExited;
    int m_iRetStatus;
    std::string m_ReadBuffer;
    std::string m_Path;
    
    void InitTerm(void);
    void CreatePTY(void);
    void GrantPT(void);
    void UnlockPT(void);
    bool SetupChildSlave(int fd);
    int Kill(bool canthrow);
    void Close(void);

protected:
    bool CheckPidExited(bool block, bool canthrow=true);
    int GetPTYFD(void) const { return m_iPTYFD; }
    std::string GetTTYName(void) const { return m_TTYName; }
    pid_t GetChildPid(void) const { return m_ChildPid; }
    
public:
    enum EReadStatus { READ_AGAIN, READ_LINE, READ_EOF, READ_LAST };
    
    CPseudoTerminal(void);
    virtual ~CPseudoTerminal(void);
    
    void SetPath(const std::string &p) { m_Path = p; }
    void Exec(const std::string &command);
    bool CheckForData(void);
    int GetRetStatus(void);
    void Abort(bool canthrow=true);
    EReadStatus ReadLine(std::string &out, bool onlyline=true);
    bool IsValid(void);
    bool CommandFinished(void);
    
    operator void *(void);
};

#endif
