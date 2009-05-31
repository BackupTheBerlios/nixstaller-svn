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

#ifndef SUTERM_H
#define SUTERM_H

#include "main/main.h"
#include "main/pseudoterminal.h"

class CSuTerm: public CPseudoTerminal
{
    enum ESuType { TYPE_SU, TYPE_SUDO, TYPE_MAYBESU, TYPE_MAYBESUDO, TYPE_UNKNOWN };
    enum ESuTalkStat { SU_OK, SU_ERROR, SU_NULLPASS, SU_AUTHERROR };

    static ESuType m_SuType;

    std::string m_User;

    const char *GetSuBin(ESuType sutype) const;
    bool UsingSudo(void) { return ((m_SuType == TYPE_SUDO) || (m_SuType == TYPE_MAYBESUDO)); }
    void ConstructCommand(std::string &cmd, TStringVec &args, const std::string &runcmd,
                          const TStringVec &runargs);
    bool CheckPid(void) { return (UsingSudo() || (kill(GetChildPid(), 0) == 0)); };
    bool WaitSlave(void);
    ESuTalkStat TalkWithSU(const char *password);
    
    using CPseudoTerminal::Exec;

public:
    CSuTerm(const std::string &user = "root") : m_User(user) { }
    
    bool NeedPassword(void);
    bool TestSU(const char *password);
    void Exec(const std::string &command, const char *password);
};

#endif
