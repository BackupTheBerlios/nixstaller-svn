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

#include <fstream>
#include <sstream>
#include <limits>
#include <errno.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include "main.h"

std::list<char *> StringList;

// -------------------------------------
// Main Class
// -------------------------------------

CMain::~CMain()
{
    if (!m_Translations.empty())
    {
        for(std::map<std::string, char *>::iterator p=m_Translations.begin(); p!=m_Translations.end(); p++)
            delete [] (*p).second;
    }
    
    FreeStrings();
}

bool CMain::Init(int argc, char **argv)
{
    if (!ReadConfig())
        return false;
    
    if (m_Languages.empty())
    {
        char *s = new char[8];
        strcpy(s, "english");
        m_Languages.push_front(s);
    }
    
    m_szCurLang = m_Languages.front();

    return true;
}

void CMain::ThrowError(bool dialog, const char *error, ...)
{
    char *txt;
    const char *translated = GetTranslation(error);
    va_list v;
    
    va_start(v, error);
        vasprintf(&txt, translated, v);
    va_end(v);

    if (dialog) Warn(txt);
    else { fprintf(stderr, GetTranslation("Error: %s"), txt); fprintf(stderr, "\n"); }

    EndProg(true);
}

void CMain::SetUpSU(const char *msg)
{
    m_SUHandler.SetUser("root");
    m_SUHandler.SetTerminalOutput(false);

    if (m_SUHandler.NeedPassword())
    {
        while(true)
        {
            CleanPasswdString(m_szPassword);
            
            m_szPassword = GetPassword(GetTranslation(msg));
            
            // Check if password is invalid
            if (!m_szPassword)
            {
                if (ChoiceBox(GetTranslation("Root access is required to continue\nAbort installation?"),
                    GetTranslation("No"), GetTranslation("Yes"), NULL))
                    EndProg();
            }
            else
            {
                if (m_SUHandler.TestSU(m_szPassword))
                    break;

                // Some error appeared
                if (m_SUHandler.GetError() == LIBSU::CLibSU::SU_ERROR_INCORRECTPASS)
                    Warn(GetTranslation("Incorrect password given for root user\nPlease retype"));
                else
                {
                    ThrowError(true, GetTranslation("Could not use su to gain root access"
                            "Make sure you can use su(adding the current user to the wheel group may help"));
                }
            }
        }
    }
}

bool CMain::ReadLang()
{
    // Clear all translations
    if (!m_Translations.empty())
    {
        std::map<std::string, char *>::iterator p = m_Translations.begin();
        for(;p!=m_Translations.end();p++)
            delete [] (*p).second;
        m_Translations.erase(m_Translations.begin(), m_Translations.end());
    }
    
    std::ifstream file(CreateText("config/lang/%s/strings", m_szCurLang.c_str()));

    if (!file)
        return false;
    
    std::string text, srcmsg;
    bool atsrc = true;
    while (file)
    {
        std::getline(file, text);
        EatWhite(text);

        if (text.empty() || text[0] == '#')
            continue;

        if (text[0] == '[')
            GetTextFromBlock(file, text);
        
        if (atsrc)
            srcmsg = text;
        else
        {
            m_Translations[srcmsg] = new char[text.length()+1];
            text.copy(m_Translations[srcmsg], std::string::npos);
            m_Translations[srcmsg][text.length()] = 0;
        }

        atsrc = !atsrc;
    }

    return true;
}

std::string CMain::GetTranslation(std::string &s)
{
    std::map<std::string, char *>::iterator p = m_Translations.find(s);
    if (p != m_Translations.end())
        return (*p).second;
    
    // No translation found
    //debugline("WARNING: No translation for %s\n", s.c_str());
    return s;
}

char *CMain::GetTranslation(char *s)
{
    std::map<std::string, char *>::iterator p = m_Translations.find(s);
    if (p != m_Translations.end())
        return (*p).second;
    
    // No translation found
    //debugline("WARNING: No translation for %s\n", s);
    return s;
}

std::string CMain::ReadRegField(std::ifstream &file)
{
    std::string line, ret;
    std::string::size_type index = 0;
    
    std::getline(file, line);
    EatWhite(line);
    
    if (line[0] != '\"')
        return "";
    
    line.erase(0, 1); // Remove initial "
    while ((index = line.find("\\\"", index+1)) != std::string::npos)
        line.replace(index, 2, "\"");

    ret = line;
    
    while(line[line.length()-1] != '\"' && file && std::getline(file, line))
    {
        EatWhite(line);
        while ((index = line.find("\\\"")) != std::string::npos)
            line.replace(index, 2, "\"");
        ret += "\n" + line;
    }
    
    ret.erase(ret.length()-1, 1); // Remove trailing "
    return ret;
}

app_entry_s *CMain::GetAppRegEntry(const char *progname)
{
    const char *filename = GetRegConfFile(progname);
    if (!FileExists(filename))
        return NULL;

    app_entry_s *pAppEntry = new app_entry_s;
    pAppEntry->name = progname;
    
    std::ifstream file(filename);
    std::string str, field;
    
    while(file && (file >> str))
    {
        field = ReadRegField(file);
        
        if (str == "version")
            pAppEntry->version = field;
        else if (str == "description")
            pAppEntry->description = field;
        else if (str == "url")
            pAppEntry->url = field;
    }

    file.close();

    filename = GetSumListFile(progname);
    if (FileExists(filename))
    {
        std::string line, sum;

        std::ifstream sumfile(filename);
        while(sumfile)
        {
            if (!(sumfile >> sum) || !std::getline(sumfile, line))
                break;
            pAppEntry->FileSums[EatWhite(line)] = sum;
        }
    }
    
    return pAppEntry;
}

const char *CMain::GetAppRegDir()
{
    if (!m_szAppConfDir)
    {
        const char *home = getenv("HOME");
        if (!home)
            ThrowError(false, "Couldn't find out your home directory!");
        
        m_szAppConfDir = CreateText("%s/.nixstaller", home);
        
        if (mkdir(m_szAppConfDir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
            ThrowError(false, "Could not create nixstaller config directory!(%s)", strerror(errno));
    }
    
    return m_szAppConfDir;
}

const char *CMain::GetRegConfFile(const char *progname)
{
    const char *dir = CreateText("%s/%s", GetAppRegDir(), progname);
    if (mkdir(dir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
        ThrowError(false, "Could not create nixstaller app-config directory!(%s)", strerror(errno));
    return CreateText("%s/config", dir);
}

const char *CMain::GetSumListFile(const char *progname)
{
    const char *dir = CreateText("%s/%s", GetAppRegDir(), progname);
    if (mkdir(dir, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH)) && (errno != EEXIST))
        ThrowError(false, "Could not create nixstaller app-config directory!(%s)", strerror(errno));
    return CreateText("%s/list", dir);
}
