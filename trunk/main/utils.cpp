#include <archive.h>
#include <archive_entry.h>
#include <sys/stat.h>
#include "main.h"
 

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

std::string GetParameters(command_entry_s *pCommandEntry)
{
    std::string args;
    
    for(std::map<std::string, param_entry_s *>::iterator it=pCommandEntry->parameter_entries.begin();
        it!=pCommandEntry->parameter_entries.end();it++)
    {
        switch (it->second->param_type)
        {
            case PTYPE_STRING:
            case PTYPE_DIR:
            case PTYPE_LIST:
                args += " " + it->second->parameter + it->second->value;
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
    return s;
}

char *GetTranslation(char *s)
{
    std::map<std::string, char *>::iterator p = InstallInfo.translations.find(s);
    if (p != InstallInfo.translations.end()) return (*p).second;
    
    // No translation found
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
    while(!StringList.empty())
    {
        delete [] (*StringList.end());
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

