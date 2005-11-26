#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <archive.h>
#include <archive_entry.h>

#include "main.h"

install_info_s InstallInfo;
std::list<char *> StringList;

void check()
{
     printf("lib is functioning\n");
}

// Reads install data from config file
bool ReadConfig()
{
    FILE *cfgfile = fopen("config/install.cfg", "r");
    if (!cfgfile) return false;
    
    char cfgline[256];
    int index;
    bool handlecommandentry = false, handleparamentry = false;
    bool incommandentry = false, inparamentry = false;
    command_entry_s *pCommandEntry = NULL;
    param_entry_s *pParamEntry = NULL;
    std::string ParamName;
    
    while(cfgfile)
    {
        index = 0;
        cfgline[0] = 0;
        
        char c = fgetc(cfgfile);
        
        // skip any leading blanks
        while ((c == ' ') || (c == '\t')) c = fgetc(cfgfile);

        while ((c != EOF) && (c != '\r') && (c != '\n'))
        {
            if (c == '\t')  c = ' ';

            cfgline[index] = c;

            c = fgetc(cfgfile);

            // skip multiple spaces in input file
            while ((cfgline[index] == ' ') && (c == ' '))
                c = fgetc(cfgfile);

            index++;
        }

        if (c == '\r') c = fgetc(cfgfile);

        if (c == EOF)
        {
            fclose(cfgfile);
            cfgfile = NULL;
        }

        cfgline[index] = 0;

        if ((cfgline[0] == '#') || !cfgline[0]) continue;
        
        char fullline[256];
        strcpy(fullline, cfgline); // Backup full line
        
        char *arg1 = NULL, *arg2 = NULL;

        arg1 = strtok(cfgline, " ");
        arg2 = strtok(NULL, " ");

        if (handlecommandentry)
        {
            if (!strcmp(arg1, "["))
            {
                incommandentry = true;
                handlecommandentry = false;
                pCommandEntry = new command_entry_s;
            }
        }
        else if (incommandentry)
        {
            if (handleparamentry)
            {
                if (!strcmp(arg1, "["))
                {
                    inparamentry = true;
                    handleparamentry = false;
                    pParamEntry = new param_entry_s;
                    ParamName = "";
                }
            }
            else if (inparamentry)
            {
                if (!strcmp(arg1, "]"))
                {
                    inparamentry = false;
                    if (!ParamName.empty()) pCommandEntry->parameter_entries[ParamName] = pParamEntry;
                    pParamEntry = NULL;
                }
                else if (!arg2); // Following stuff needs atleast second argument
                else if (!strcasecmp(arg1, "name")) ParamName = &fullline[strlen(arg1)+1];
                else if (!strcasecmp(arg1, "parameter")) pParamEntry->parameter = &fullline[strlen(arg1)+1];
                else if (!strcasecmp(arg1, "description")) pParamEntry->description = &fullline[strlen(arg1)+1];
                else if (!strcasecmp(arg1, "defaultval"))
                    pParamEntry->defaultval = pParamEntry->value = &fullline[strlen(arg1)+1];
                else if (!strcasecmp(arg1, "type"))
                {
                    if (!strcasecmp(arg2, "string")) pParamEntry->param_type = PTYPE_STRING;
                    else if (!strcasecmp(arg2, "dir")) pParamEntry->param_type = PTYPE_DIR;
                    else if (!strcasecmp(arg2, "list")) pParamEntry->param_type = PTYPE_LIST;
                    else if (!strcasecmp(arg2, "bool")) pParamEntry->param_type = PTYPE_BOOL;
                }
                else if (!strcasecmp(arg1, "addchoice")) pParamEntry->options.push_back(arg2);
                else if (!strcasecmp(arg1, "varname")) pParamEntry->varname = arg2;
            }
            else
            {
                if (!strcmp(arg1, "]"))
                {
                    incommandentry = false;
                    InstallInfo.command_entries.push_back(pCommandEntry);
                    pCommandEntry = NULL;
                }
                else if (!strcasecmp(arg1, "addparam")) handleparamentry = true;
                else if (!arg2); // Following stuff needs atleast second argument
                else if (!strcasecmp(arg1, "needroot"))
                {
                    if (!strcasecmp(arg2, "true")) pCommandEntry->need_root = NEED_ROOT;
                    else if (!strcasecmp(arg2, "false")) pCommandEntry->need_root = NO_ROOT;
                    else if (!strcasecmp(arg2, "dependson"))
                    {
                        char *arg3 = strtok(NULL, " ");
                        if (arg3)
                        {
                            pCommandEntry->need_root = DEPENDED_ROOT;
                            pCommandEntry->dep_param = arg3;
                        }
                    }
                }
                else if (!strcasecmp(arg1, "command")) pCommandEntry->command = &fullline[8];
                else if (!strcasecmp(arg1, "description")) pCommandEntry->description = &fullline[12];
            }
        }
        else
        {
            if (!strcasecmp(arg1, "commandentry")) handlecommandentry = true;
            else if (!arg2); // Following stuff needs atleast second argument
            if (!strcasecmp(arg1, "version")) InstallInfo.version = atoi(arg2);
            else if (!strcasecmp(arg1, "appname"))
            {
                strncpy(InstallInfo.program_name, arg2, 128);
                InstallInfo.program_name[128] = 0;
            }
            else if (!strcasecmp(arg1, "archtype"))
            {
                if (!strcasecmp(arg2, "gzip")) InstallInfo.archive_type = ARCH_GZIP;
                else if (!strcasecmp(arg2, "bzip2")) InstallInfo.archive_type = ARCH_BZIP2;
            }
            else if (!strcasecmp(arg1, "languages"))
            {
                char *lang = arg2;
                while (lang)
                {
                    char *s = new char[strlen(cfgline)+1];
                    strcpy(s, lang);
                    InstallInfo.languages.push_back(s);
                    lang = strtok(NULL, " ");
                }
            }
            else if (!strcasecmp(arg1, "installdir"))
            {
                if (!strcasecmp(arg2, "select")) InstallInfo.dest_dir_type = DEST_SELECT;
                else if (!strcasecmp(arg2, "temp")) InstallInfo.dest_dir_type = DEST_TEMP;
                else if (!strcasecmp(arg2, "default"))
                {
                    char *arg3 = strtok(NULL, " ");
                    if (arg3) { InstallInfo.dest_dir_type = DEST_DEFAULT; InstallInfo.dest_dir = arg3; }
                }
            }
        }
    }
    
    printf("Comp entries:\n");
    for (std::list<command_entry_s *>::iterator p=InstallInfo.command_entries.begin();p!=InstallInfo.command_entries.end();p++)
    {
        printf("Need root: %d\n", (*p)->need_root);
        printf("Command: %s\n", (*p)->command.c_str());
        printf("Description: %s\n", (*p)->description.c_str());
        printf("Depends on param: %s\n", (*p)->dep_param.c_str());
        printf("Params:\n");
        for (std::map<std::string, param_entry_s *>::iterator
             p2=(*p)->parameter_entries.begin();p2!=(*p)->parameter_entries.end();p2++)
        {
            printf("\tName: %s\n\tType: %d\n\tParameter: %s\n\tDefault: %s\n\t"
                    "Description: %s\n\tVarname: %s\n", (*p2).first.c_str(), (*p2).second->param_type,
                                                        (*p2).second->parameter.c_str(),
                                                        (*p2).second->defaultval.c_str(), (*p2).second->description.c_str(),
                                                        (*p2).second->varname.c_str());
            printf("\tOptions: ");
            for (std::list<std::string>::iterator p3=(*p2).second->options.begin();p3!=(*p2).second->options.end();p3++)
                printf("%s ", p3->c_str());
            printf("\n");
        }
    }
    return true;
}

bool MainInit(int argc, char *argv[])
{
    if (!ReadConfig()) return false;
    
    if (getcwd(InstallInfo.arch_name, sizeof(InstallInfo.arch_name)) == 0)
        throwerror(false, "Could not read current directory");
    strcat(InstallInfo.arch_name, "/instarchive");
    
    if (InstallInfo.dest_dir_type == DEST_TEMP)
    {
        char dir[1024];
        if (getcwd(dir, sizeof(dir))) InstallInfo.dest_dir = dir;
        else throwerror(false, "Could not read current directory");
    }
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
    
    if (!InstallInfo.languages.empty())
    {
        std::list<char*>::iterator p = InstallInfo.languages.begin();
        for(;p!=InstallInfo.languages.end();p++) delete [] *p;
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

// Extract gzipped tar file. Returns how much percent is done.
float ExtractArchive(char *curfile)
{
    static archive *arch = NULL;
    archive_entry *entry = NULL;
    static int size;
    static float percent;

    if (!arch) // Does the file needs to be opened?
    {
        size = ArchSize(InstallInfo.arch_name);
        percent = 0.0f;
        
        arch = archive_read_new();
        archive_read_support_compression_all(arch);
        archive_read_support_format_all(arch);
        archive_read_open_file(arch, InstallInfo.arch_name, 512);
    }
    
    int status = archive_read_next_header(arch, &entry);
    if (status == ARCHIVE_OK)
    {
        strcpy(curfile, archive_entry_pathname(entry));
        percent += ((float)archive_entry_size(entry)/(float)size)*100.0f;
        archive_read_extract(arch, entry, (ARCHIVE_EXTRACT_OWNER | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_FFLAGS));
        return percent;
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
    
    if (InstallInfo.cur_lang == "english") return true; // No need to translate...
    
    char filename[64];
    sprintf(filename, "config/lang/%s/strings", InstallInfo.cur_lang.c_str());
    FILE *langfile = fopen(filename, "r");
    
    if (!langfile) return false;
    
    char cfgline[1024];
    int index;
    std::string engtxt;
    bool english = true;
    
    while(langfile)
    {
        index = 0;
        cfgline[0] = 0;
        
        char c = fgetc(langfile);
        
        // skip any leading blanks
        while ((c == ' ') || (c == '\t')) c = fgetc(langfile);

        while ((c != EOF) && (c != '\r') && (c != '\n'))
        {
            if (c == '\t')  c = ' ';

            cfgline[index] = c;

            c = fgetc(langfile);

            // skip multiple spaces in input file
            while ((cfgline[index] == ' ') && (c == ' '))
                c = fgetc(langfile);

            index++;
        }

        if (c == '\r') c = fgetc(langfile);

        if (c == EOF)
        {
            fclose(langfile);
            langfile = NULL;
        }

        cfgline[index] = 0;

        if ((cfgline[0] == '#') || !cfgline[0]) continue;
        
        if (english) engtxt = cfgline;
        else
        {
            InstallInfo.translations[engtxt] = new char[strlen(cfgline)+1];
            strcpy(InstallInfo.translations[engtxt], cfgline);
        }
        english = !english;
    }
    return true;
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
        std::list<char *>::iterator it = find(StringList.begin(), StringList.end(), txt);
        if (it != StringList.end())
            return *it;
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

param_entry_s *GetParamVar(const std::string &str)
{
    for (std::list<command_entry_s *>::iterator it=InstallInfo.command_entries.begin(); it!=InstallInfo.command_entries.end();
         it++)
    {
        for (std::map<std::string, param_entry_s *>::iterator it2=(*it)->parameter_entries.begin();
             it2!=(*it)->parameter_entries.end(); it2++)
        {
            if (it2->second->varname == str) return it2->second;
        }
    }
    return NULL;
}

