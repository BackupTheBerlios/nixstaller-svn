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

#include <stdlib.h>

#include "main/main.h"
#include "suterm.h"
#include "utils.h"

std::list<char *> StringList;
extern std::map<std::string, char *> Translations;

// Required by exceptions.h
const char *GetExTranslation(const char *s)
{
    return GetTranslation(s);
}

void PrintIntro(const std::string &path)
{
    std::ifstream file(JoinPath(path, "start").c_str());
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

bool MKDirNeedsRoot(std::string dir)
{
    MakeAbsolute(dir);
    
    const TSTLStrSize length = dir.length();
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
    while ((end != std::string::npos) && (end < length));
    
    return needroot;
}

void MKDirRecRoot(std::string dir, CSuTerm *suterm, const char *passwd)
{
    MakeAbsolute(dir);
    
    try
    {
        suterm->Exec("mkdir -p " + dir, passwd);
        while (*suterm && !suterm->CommandFinished())
            ; // Wait till command is finished
    }
    catch (Exceptions::CExIO &)
    {
        // Usually we don't care about the detials, so just translate exception.
        throw Exceptions::CExRootMKDir(dir.c_str());
    }
}

