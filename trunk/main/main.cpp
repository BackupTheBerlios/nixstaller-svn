/*
    Copyright (C) 2006 by Rick Helmus (rhelmus@gmail.com)

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
#include <archive.h>
#include <archive_entry.h>
#include "main.h"

install_info_s InstallInfo;
std::list<char *> StringList;

bool MainInit(int argc, char *argv[])
{
    printf("Nixstaller version 0.1, Copyright (C) 2006 of Rick Helmus\n"
           "Nixstaller comes with ABSOLUTELY NO WARRANTY.\n"
           "This is free software, and you are welcome to redistribute it\n"
           "under certain conditions; see the about section for details.\n");
    
    if (!ReadConfig())
        return false;

    if (argc < 2)
        return false;
    
    InstallInfo.os = argv[1];
    
    char curdir[1024];
    if (getcwd(curdir, sizeof(curdir)) == 0)
        throwerror(false, "Could not read current directory");

    InstallInfo.own_dir = curdir;
    
    if (InstallInfo.dest_dir_type == DEST_TEMP)
        InstallInfo.dest_dir = curdir;
    else if (InstallInfo.dest_dir_type == DEST_SELECT)
    {
        const char *env = getenv("HOME");
        if (env) InstallInfo.dest_dir = env;
        else InstallInfo.dest_dir = "/";
    }
    
    if (InstallInfo.languages.empty())
    {
        char *s = new char[8];
        strcpy(s, "english");
        InstallInfo.languages.push_front(s);
        InstallInfo.cur_lang = "english";
    }
    return true;
}

void MainEnd()
{
    if (!InstallInfo.translations.empty())
    {
        std::map<std::string, char *>::iterator p = InstallInfo.translations.begin();
        for(;p!=InstallInfo.translations.end();p++) delete [] (*p).second;
    }
    
    if (!InstallInfo.command_entries.empty())
    {
        std::list<command_entry_s *>::iterator p = InstallInfo.command_entries.begin();
        for(;p!=InstallInfo.command_entries.end();p++)
        {
            if (!(*p)->parameter_entries.empty())
            {
                for (std::map<std::string, param_entry_s *>::iterator p2=(*p)->parameter_entries.begin();
                     p2!=(*p)->parameter_entries.end();p2++)
                    delete (*p2).second;
            }
            delete *p;
        }
    }
    
    FreeStrings();
}

// Reads install data from config file
bool ReadConfig()
{
    const int maxread = std::numeric_limits<std::streamsize>::max();
    std::ifstream file("config/install.cfg");
    std::string str, line, tmp, ParamName;
    std::string::size_type strpos;
    bool incommandentry = false, inparamentry = false;
    command_entry_s *pCommandEntry = NULL;
    param_entry_s *pParamEntry = NULL;
    
    while(file)
    {
        // Read one line at a time...this makes sure that we don't start reading at a new line if the user 'forgot'
        // to enter a value after the first command/variabele
        std::getline(file, line);
        std::istringstream strstrm(line); // Use a stringstream to easily read from this line
        
        if (!(strstrm >> str))
            continue;

        if (str[0] == '#') // Comment, skip
            continue;

        if (incommandentry)
        {
            if (inparamentry)
            {
                if (str[0] == ']')
                {
                    if (!ParamName.empty())
                        pCommandEntry->parameter_entries[ParamName] = pParamEntry;
                    pParamEntry = NULL;
                    inparamentry = false;
                }
                else if (str == "name")
                {
                    std::getline(strstrm, tmp);
                    ParamName = EatWhite(tmp);
                }
                else if (str == "parameter")
                {
                    std::getline(strstrm, tmp);
                    pParamEntry->parameter = EatWhite(tmp);
                }
                else if (str == "description")
                {
                    std::getline(strstrm, tmp);
                    pParamEntry->description = EatWhite(tmp);
                }
                else if (str == "defaultval")
                {
                    std::getline(strstrm, tmp);
                    pParamEntry->defaultval = EatWhite(tmp);
                    pParamEntry->value = pParamEntry->defaultval;
                }
                else if (str == "varname")
                {
                    std::getline(strstrm, tmp);
                    pParamEntry->varname = EatWhite(tmp);
                }
                else if (str == "addchoice")
                {
                    std::getline(strstrm, tmp);
                    pParamEntry->options.push_back(EatWhite(tmp));

                }
                else if (str == "type")
                {
                    std::string type;
                    strstrm >> type;
                    if (type == "string")
                        pParamEntry->param_type = PTYPE_STRING;
                    else if (type == "dir")
                        pParamEntry->param_type = PTYPE_DIR;
                    else if (type == "list")
                        pParamEntry->param_type = PTYPE_LIST;
                    else if (type == "bool")
                        pParamEntry->param_type = PTYPE_BOOL;
                }
            }
            else
            {
                if (str[0] == ']')
                {
                    InstallInfo.command_entries.push_back(pCommandEntry);
                    pCommandEntry = NULL;
                    incommandentry = false;
                }
                else if (str == "command")
                {
                    std::getline(strstrm, tmp);
                    pCommandEntry->command = EatWhite(tmp);
                }
                else if (str == "description")
                {
                    std::getline(strstrm, tmp);
                    pCommandEntry->description = EatWhite(tmp);
                }
                else if (str == "needroot")
                {
                    std::string s;
                    strstrm >> s;
                    if (s == "true")
                        pCommandEntry->need_root = NEED_ROOT;
                    else if (s == "false")
                        pCommandEntry->need_root = NO_ROOT;
                    else if (s == "dependson")
                    {
                        if (strstrm >> pCommandEntry->dep_param)
                            pCommandEntry->need_root = DEPENDED_ROOT;
                    }
                }
                else if (str == "exitonfailure")
                {
                    std::string ex;
                    strstrm >> ex;
                    if (ex == "true")
                        pCommandEntry->exit_on_failure = true;
                    else if (ex == "false")
                        pCommandEntry->exit_on_failure = false;
                }
                else if (str == "path")
                {
                    std::getline(strstrm, tmp);
                    pCommandEntry->path = EatWhite(tmp);

                }
                else if (str == "addparam")
                {
                    file.ignore(maxread, '[');
                    inparamentry = true;
                    pParamEntry = new param_entry_s;
                    ParamName.clear();
                }
            }
        }
        else
        {
            if (str == "appname")
            {
                std::getline(strstrm, tmp);
                InstallInfo.program_name = EatWhite(tmp);

            }
            else if (str == "version") // unused
            {
                std::getline(strstrm, tmp);
                InstallInfo.version = EatWhite(tmp);
            }
            else if (str == "archtype")
            {
                std::string type;
                strstrm >> type;
                if (type == "gzip")
                    InstallInfo.archive_type = ARCH_GZIP;
                else if (type == "bzip2")
                    InstallInfo.archive_type = ARCH_BZIP2;
            }
            else if (str == "installdir")
            {
                std::string type;
                strstrm >> type;
                if (type == "select")
                    InstallInfo.dest_dir_type = DEST_SELECT;
                if (type == "temp")
                    InstallInfo.dest_dir_type = DEST_TEMP;
                else if (type == "default")
                {
                    if (strstrm >> InstallInfo.dest_dir)
                        InstallInfo.dest_dir_type = DEST_DEFAULT;
                }
            }
            else if (str == "languages")
            {
                std::string lang;
                while (strstrm >> lang)
                    InstallInfo.languages.push_back(lang);
            }
            else if (str == "intropic")
            {
                std::getline(strstrm, tmp);
                InstallInfo.intropicname = EatWhite(tmp);
            }
            else if (str == "commandentry")
            {
                file.ignore(maxread, '[');
                incommandentry = true;
                pCommandEntry = new command_entry_s;
            }
        }
    }

#ifndef RELEASE

    debugline("appname: %s\n", InstallInfo.program_name.c_str());
    debugline("version: %s\n", InstallInfo.version.c_str());
    debugline("archtype: %d\n", InstallInfo.archive_type);
    debugline("installdir: %s\n", InstallInfo.dest_dir.c_str());
    debugline("dir type: %d\n", InstallInfo.dest_dir_type);
    debugline("languages: ");
    for (std::list<std::string>::iterator it=InstallInfo.languages.begin(); it!=InstallInfo.languages.end(); it++)
        debugline("%s ", it->c_str());
    debugline("\n");

    debugline("Comp entries:\n");
    for (std::list<command_entry_s *>::iterator p=InstallInfo.command_entries.begin();p!=InstallInfo.command_entries.end();p++)
    {
        debugline("Need root: %d\n", (*p)->need_root);
        debugline("Command: %s\n", (*p)->command.c_str());
        debugline("Description: %s\n", (*p)->description.c_str());
        debugline("Depends on param: %s\n", (*p)->dep_param.c_str());
        debugline("Exit on failure: %d\n", (*p)->exit_on_failure);
        debugline("Params:\n");
        for (std::map<std::string, param_entry_s *>::iterator
             p2=(*p)->parameter_entries.begin();p2!=(*p)->parameter_entries.end();p2++)
        {
            debugline("\tName: %s\n\tType: %d\n\tParameter: %s\n\tDefault: %s\n\t"
                      "Description: %s\n\tVarname: %s\n", (*p2).first.c_str(), (*p2).second->param_type,
                      (*p2).second->parameter.c_str(),
                      (*p2).second->defaultval.c_str(), (*p2).second->description.c_str(),
                      (*p2).second->varname.c_str());
            debugline("\tOptions: ");
            for (std::list<std::string>::iterator p3=(*p2).second->options.begin();p3!=(*p2).second->options.end();p3++)
                debugline("%s ", p3->c_str());
            debugline("\n");
        }
    }

#endif

    return true;
}

// Extract gzipped tar file. Returns how much percent is done.
float ExtractArchive(std::string &curfile)
{
    static archive *arch = NULL;
    archive_entry *entry = NULL;
    static int size;
    static float percent;
    static std::list<char *> archlist;

    if (!arch) // Does the file need to be opened?
    {
        char *archnameall = CreateText("%s/instarchive_all", InstallInfo.own_dir.c_str());
        char *archnameos = CreateText("%s/instarchive_%s", InstallInfo.own_dir.c_str(), InstallInfo.os.c_str());
        
        size = 0;
        archlist.clear();
        
        if (FileExists(archnameall))
        {
            size += ArchSize(archnameall);
            archlist.push_back(archnameall);
        }
        if (FileExists(archnameos))
        {
            size += ArchSize(archnameos);
            archlist.push_back(archnameos);
        }
        
        if (archlist.empty())
            return 100;
        
        percent = 0.0f;
        
        arch = archive_read_new();
        archive_read_support_compression_all(arch);
        archive_read_support_format_all(arch);
        archive_read_open_file(arch, archlist.front(), 512);
    }
    
    int status = archive_read_next_header(arch, &entry);
    
    if ((status == ARCHIVE_EOF) && !archlist.empty())
    {
        archlist.pop_front();
        archive_read_finish(arch);
        arch = archive_read_new();
        archive_read_support_compression_all(arch);
        archive_read_support_format_all(arch);
        archive_read_open_file(arch, archlist.front(), 512);
        
        status = archive_read_next_header(arch, &entry);
    }
        
    if (status == ARCHIVE_OK)
    {
        curfile = archive_entry_pathname(entry);
        percent += ((float)archive_entry_size(entry)/(float)size)*100.0f;
        archive_read_extract(arch, entry, (ARCHIVE_EXTRACT_OWNER | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_FFLAGS));
        return percent/archlist.size();
    }
    
    archive_read_finish(arch);
    arch = NULL;
    
    return (status == ARCHIVE_EOF) ? 100 : -1;
}

bool ReadLang()
{
    // Clear all translations
    if (!InstallInfo.translations.empty())
    {
        std::map<std::string, char *>::iterator p = InstallInfo.translations.begin();
        for(;p!=InstallInfo.translations.end();p++) delete [] (*p).second;
        InstallInfo.translations.erase(InstallInfo.translations.begin(), InstallInfo.translations.end());
    }
    
    std::ifstream file(CreateText("config/lang/%s/strings", InstallInfo.cur_lang.c_str()));

    if (!file)
        return false;

    std::string tmp, text, srcmsg;
    bool atsrc = true;
    while (file)
    {
        std::getline(file, tmp);
        text = EatWhite(tmp);

        if (text.empty() || text[0] == '#')
            continue;

        if (atsrc)
            srcmsg = text;
        else
        {
            InstallInfo.translations[srcmsg] = new char[text.length()+1];
            text.copy(InstallInfo.translations[srcmsg], std::string::npos);
            InstallInfo.translations[srcmsg][text.length()] = 0;
        }

        atsrc = !atsrc;
    }
    
    return true;
}
