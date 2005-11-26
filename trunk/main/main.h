#ifndef MAIN_H
#define MAIN_H

#include "libsu.h"

#include <stdarg.h>
#include <string>
#include <list>
#include <map>

enum EArchiveType { ARCH_GZIP, ARCH_BZIP2 };
enum ENeedRoot { NO_ROOT, NEED_ROOT, DEPENDED_ROOT };
enum EParamType { PTYPE_STRING, PTYPE_DIR, PTYPE_LIST, PTYPE_BOOL };
enum EDestDirType { DEST_TEMP, DEST_SELECT, DEST_DEFAULT };

struct param_entry_s
{
    std::string parameter, defaultval, description, varname;
    std::list<std::string> options;
    std::string value;
    EParamType param_type;
    param_entry_s(void) : param_type(PTYPE_BOOL) { };
};

struct command_entry_s
{
    ENeedRoot need_root;
    std::string command, description;
    std::map<std::string, param_entry_s *> parameter_entries;
    std::string dep_param;
    command_entry_s(void) : need_root(NO_ROOT) { };
};

struct install_info_s
{
    int version;
    char program_name[129];
    char arch_name[2048];
    std::string dest_dir;
    std::list<char *> languages;
    std::string cur_lang;
    std::map<std::string, char *> translations;
    EArchiveType archive_type;
    EDestDirType dest_dir_type;
    std::list<command_entry_s *> command_entries;
    
    install_info_s(void) : version(1), archive_type(ARCH_GZIP), dest_dir_type(DEST_SELECT) { strcpy(program_name, "foo"); };
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
param_entry_s *GetParamVar(const std::string &str);
void throwerror(bool dialog, const char *error, ...); // Frontend depended, therefor defined in there

#endif 
