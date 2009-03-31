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

#ifndef FR_UTILS_H
#define FR_UTILS_H

#include <fstream>
#include <list>
#include "libsu/libsu.h"
#include "main/exception.h"

extern std::list<char *> StringList; // List of all strings created by CreateText

void PrintIntro(void);
char *CreateText(const char *s, ...);
inline char *MakeCString(const std::string &s) { return CreateText(s.c_str()); };
void FreeStrings(void);
void FreeTranslations(void);
void *guaranteed_memset(void *v,int c,size_t n);
void CleanPasswdString(char *str);
void GetTextFromBlock(std::ifstream &file, std::string &text);
std::string GetTranslation(const std::string &s);
const char *GetTranslation(const char *s);
inline char *MakeTranslation(const std::string &s) { return MakeCString(GetTranslation(s)); }
inline char *MakeTranslation(const char *s) { return CreateText(GetTranslation(s)); }
bool MKDirNeedsRoot(std::string dir);
void MKDirRecRoot(std::string dir, LIBSU::CLibSU &suhandler, const char *passwd);

namespace Exceptions {

class CExCURL: public CExMessage
{
public:
    CExCURL(const char *msg) : CExMessage(msg) { }
    virtual const char *what(void) throw() { return FormatText(GetTranslation("Error during file transfer: %s"), Message()); };
};

class CExRootMKDir: public CExIO
{
    char m_szDir[1024];
    
public:
    CExRootMKDir(const char *dir) { StoreString(dir, m_szDir, sizeof(m_szDir)); };
    virtual const char *what(void) throw()
    { return FormatText(GetTranslation("Could not create directory \'%s\'"), m_szDir); };
};

class CExFrontend: public CExMessage
{
public:
    CExFrontend(const char *msg) : CExMessage(msg) { };
};

}

#endif
