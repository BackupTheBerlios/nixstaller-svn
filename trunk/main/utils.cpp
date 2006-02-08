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

#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include "main.h"

#ifdef WITH_LIB_ARCHIVE

#include <archive.h>
#include <archive_entry.h>

// Returns uncompressed file size of a gzipped tar file
int ArchSize(const char *archname)
{
    archive *arch;
    archive_entry *entry;
    int size = 0;

    arch = archive_read_new();
    archive_read_support_compression_all(arch);
    archive_read_support_format_all(arch);
    archive_read_open_file(arch, archname, 512);
    
    while (archive_read_next_header(arch, &entry) == ARCHIVE_OK)
    {
        size += archive_entry_size(entry);
        archive_read_data_skip(arch);
    }
    
    archive_read_finish(arch);
   
    return size;
}
#else
void GetArchiveInfo(const char *archname, std::map<std::string, unsigned int> &archfilesizes, unsigned int &totalsize)
{
    /*
    totalsize = 0;
    archfilesizes.clear();
    
    std::string command = "cat " + std::string(archname);

    if (InstallInfo.archive_type == ARCH_GZIP)
        command += " | gzip -cd | tar tvf -";
    else if (InstallInfo.archive_type == ARCH_BZIP2)
        command += " | bzip2 -d | tar tvf -";
    
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe)
    {
        char line[1024];
        while (fgets(line, sizeof(line), pipe))
        {
            std::istringstream strstrm(line);
            short s;
            unsigned int size;
            std::string tmp, fname;
            for (s=0;s<9 && strstrm;s++)
            {
                if (s==4)
                    strstrm >> size;
                else if (s==8)
                    strstrm >> fname;
                else
                    strstrm >> tmp;
            }
            archfilesizes[fname] = size;
            totalsize += size;
        }
        pclose(pipe);
}*/
    char *fname = CreateText("%s.sizes", archname);
    std::ifstream file(fname);
    std::string arfilename;
    unsigned int size;
    
    if (!file) printf("Could not open %s\n", fname);
    while(file)
    {
        if (file >> size >> arfilename)
        {
            archfilesizes[arfilename] = size;
            totalsize += size;
            printf("%d\t%s\n", size, arfilename.c_str());
        }
    }
}
#endif

std::string GetParameters(command_entry_s *pCommandEntry)
{
    std::string args, param;
    std::string::size_type pos;

    for(std::map<std::string, param_entry_s *>::iterator it=pCommandEntry->parameter_entries.begin();
        it!=pCommandEntry->parameter_entries.end();it++)
    {
        switch (it->second->param_type)
        {
            case PTYPE_STRING:
            case PTYPE_DIR:
            case PTYPE_LIST:
                param = it->second->parameter;
                pos = param.find("%s");
                if (pos != std::string::npos)
                    param.replace(pos, 2, it->second->value);
                args += " " + param;
                break;
            case PTYPE_BOOL:
                if (it->second->value == "true") args += " " + it->second->parameter;
                break;
        }
    }
    return args;
}

std::string GetTranslation(std::string &s)
{
    std::map<std::string, char *>::iterator p = InstallInfo.translations.find(s);
    if (p != InstallInfo.translations.end()) return (*p).second;
    
    // No translation found
    //debugline("WARNING: No translation for %s\n", s.c_str());
    return s;
}

char *GetTranslation(char *s)
{
    std::map<std::string, char *>::iterator p = InstallInfo.translations.find(s);
    if (p != InstallInfo.translations.end()) return (*p).second;
    
    // No translation found
    //debugline("WARNING: No translation for %s\n", s);
    return s;
}

char *CreateText(const char *s, ...)
{
    static char txt[2048]; // Should be enough ;)
    va_list v;
    
    va_start(v, s);
    vsprintf(txt, s, v);
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
    
    char *output = new char[strlen(txt)+1];
    strcpy(output, txt);
    StringList.push_front(output);
    return output;
}

void FreeStrings()
{
    debugline("freeing %d strings....\n", StringList.size());
    while(!StringList.empty())
    {
        debugline("STRING: %s\n", StringList.back());
        delete [] StringList.back();
        StringList.pop_back();
    }
}

param_entry_s *GetParamByName(std::string str)
{
    for (std::list<command_entry_s *>::iterator it=InstallInfo.command_entries.begin(); it!=InstallInfo.command_entries.end();
         it++)
    {
        if ((*it)->parameter_entries.empty()) continue;
        return ((*it)->parameter_entries.find(str))->second;
    }
    return NULL;
}

param_entry_s *GetParamByVar(std::string str)
{
    for (std::list<command_entry_s *>::iterator it=InstallInfo.command_entries.begin(); it!=InstallInfo.command_entries.end();
         it++)
    {
        if ((*it)->parameter_entries.empty()) continue;
        for (std::map<std::string, param_entry_s *>::iterator it2=(*it)->parameter_entries.begin();
             it2!=(*it)->parameter_entries.end(); it2++)
        {
            if (it2->second->varname == str)
                return it2->second;
        }
    }
    return NULL;
}

const char *GetParamDefault(param_entry_s *pParam)
{
    if (pParam->param_type == PTYPE_BOOL)
    {
        if (pParam->defaultval == "true")
            return GetTranslation("Enabled");
        else
            return GetTranslation("Disabled");
    }
    return GetTranslation(pParam->defaultval.c_str());
}

const char *GetParamValue(param_entry_s *pParam)
{
    if (pParam->param_type == PTYPE_BOOL)
    {
        if (pParam->value == "true")
            return GetTranslation("Enabled");
        else
            return GetTranslation("Disabled");
    }
    return GetTranslation(pParam->value.c_str());
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

// Incase dir does not exist, it will search for the first valid top directory
std::string GetFirstValidDir(const std::string &dir)
{
    if ((dir[0] == '/') && (dir.length() == 1))
        return dir; // Root dir given

    if (FileExists(dir))
        return dir;
    
    std::string subdir = dir;

    // Remove trailing /
    if (subdir[subdir.length()-1] == '/')
        subdir.erase(subdir.length()-1, 1);
    
    std::string::size_type pos;
    do
    {
        pos = subdir.rfind('/');
        if (pos == std::string::npos)
        {
            // No absolute path given...just return current dir
            char curdir[256];
            if (getcwd(curdir, sizeof(curdir)) == 0)
                return "/";
            return curdir;
        }
        else if (pos == 0) // Reached the root dir('/')
            return "/";
        subdir.erase(pos);
    }
    while (!FileExists(subdir));
    return subdir;
}

// Used for cleaning password strings
void CleanPasswdString(char *str)
{
    if (str)
    {
        for (short s=0;s<strlen(str);s++) str[s] = 0;
        free(str);
    }
}

std::string &EatWhite(std::string &str, bool skipnewlines)
{
    const char *filter = (skipnewlines) ? " \t" : " \t\r\n";
    std::string::size_type pos = str.find_first_not_of(filter);

    if (pos != std::string::npos)
        str.erase(0, pos);
    
    pos = str.find_last_not_of(filter);

    if (pos != std::string::npos)
        str.erase(pos+1);
    //return ret.substr(fpos, (lpos==std::string::npos) ? std::string::npos : ((lpos-fpos)+1));
    return str;
}

// Put every line from a string in a list
void MakeStringList(const std::string &str, std::vector<char *> &strlist)
{
    std::istringstream strstrm(str);
    std::string line;
    while(strstrm && std::getline(strstrm, line))
        strlist.push_back(MakeCString(line));
}

// Put every line from a string in a list (C strings)
void MakeStringList(const char *str, std::vector<char *> &strlist)
{
    std::istringstream strstrm(str);
    std::string line;
    while(strstrm && std::getline(strstrm, line))
        strlist.push_back(MakeCString(line));
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
