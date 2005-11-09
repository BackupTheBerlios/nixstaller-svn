#ifndef MAIN_H
#define MAIN_H

#include "libsu.h"

#include <string>
#include <list>
#include <map>

enum EArchiveType { ARCH_GZIP, ARCH_BZIP2 };
enum EInstallType { INST_SIMPLE, INST_COMPILE };

struct command_entry_s
{
    bool need_root;
    std::string command, description;
    
    struct param_entry_s
    {
        std::string parameter, defaultval, description;
        enum EParamType { PTYPE_STRING, PTYPE_LIST, PTYPE_BOOL } param_type;
        std::list<std::string> options;
        std::string value;
        param_entry_s(void) : param_type(PTYPE_BOOL) { };
    };
    
    std::map<std::string, param_entry_s *> parameter_entries;
    
    command_entry_s(void) : need_root(false) { };
};

struct install_info_s
{
    int version;
    char program_name[129];
    char arch_name[2048];
    char dest_dir[2048];
    std::list<char *> languages;
    std::string cur_lang;
    std::map<std::string, char *> translations;
    EArchiveType archive_type;
    EInstallType install_type;
    std::list<command_entry_s *> command_entries;
    bool need_file_dialog;
    
    install_info_s(void) : version(1), archive_type(ARCH_GZIP), install_type(INST_SIMPLE),
                           need_file_dialog(true) { strcpy(program_name, "foo"); };
};

extern install_info_s InstallInfo;
extern std::list<char *> StringList; // List of all strings created by CreateText, for easy removal :)

void check(void);
bool MainInit(int argc, char *argv[]);
void MainEnd(void);
float ExtractArchive(char *curfile);
bool ReadLang(void);
std::string GetParameters(command_entry_s *pCommandEntry);
std::string GetTranslation(std::string &s);
char *GetTranslation(char *s);
inline char *GetTranslation(const char *s) { return GetTranslation(const_cast<char *>(s)); };
char *CreateText(const char *s, ...);
void FreeStrings(void);

#endif 
