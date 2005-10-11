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
        
        char *arg1 = NULL, *arg2 = NULL;

        arg1 = strtok(cfgline, " ");
        arg2 = strtok(NULL, " ");

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
                InstallInfo.languages.push_back(std::string(lang));
                lang = strtok(NULL, " ");
            }
        }
    }
    return true;
}

bool MainInit(int argc, char *argv[])
{
    if (!ReadConfig()) return false;
    if (argc < 2) return false;
    
    if (getcwd(InstallInfo.arch_name, sizeof(InstallInfo.arch_name)) == 0) strcpy(InstallInfo.arch_name, ".");
    strcat(InstallInfo.arch_name, "/instarchive.");
    
    switch(InstallInfo.archive_type)
    {
        case ARCH_GZIP: strcat(InstallInfo.arch_name, "tar.gz"); break;
        case ARCH_BZIP2: strcat(InstallInfo.arch_name, "tar.bz2"); break;
    }
    
    strcpy(InstallInfo.dest_dir, argv[1]);
    
    if (InstallInfo.languages.empty() ||
        (find(InstallInfo.languages.begin(), InstallInfo.languages.end(), "english") == InstallInfo.languages.end()))
        InstallInfo.languages.push_front("english");
    return true;
}

void MainEnd()
{
    // Clear all translations
    std::map<std::string, char *>::iterator p = InstallInfo.translations.begin();
    for(;p!=InstallInfo.translations.end();p++) delete [] (*p).second;
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
    if (InstallInfo.cur_lang == "english") return true; // No need to translate...
    
    char filename[64];
    sprintf(filename, "config/lang/%s.lng", InstallInfo.cur_lang.c_str());
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
