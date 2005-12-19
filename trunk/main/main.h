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
    std::string version;
    std::string program_name;
    std::string intropicname;
    std::string own_dir;
    std::string dest_dir;
    std::list<std::string> languages;
    std::string cur_lang;
    std::map<std::string, char *> translations;
    EArchiveType archive_type;
    EDestDirType dest_dir_type;
    std::list<command_entry_s *> command_entries;
    
    install_info_s(void) : archive_type(ARCH_GZIP), dest_dir_type(DEST_SELECT) { };
};

extern install_info_s InstallInfo;
extern std::list<char *> StringList; // List of all strings created by CreateText, for easy removal :)

bool MainInit(int argc, char *argv[]);
void MainEnd(void);
bool ReadConfig(void);
int ArchSize(const char *archname);
float ExtractArchive(std::string &curfile);
bool ReadLang(void);
std::string GetParameters(command_entry_s *pCommandEntry);
std::string GetTranslation(std::string &s);
char *GetTranslation(char *s);
inline char *GetTranslation(const char *s) { return GetTranslation(const_cast<char *>(s)); };
char *CreateText(const char *s, ...);
void FreeStrings(void);
param_entry_s *GetParamByName(std::string str);
param_entry_s *GetParamByVar(std::string str);
const char *GetParamDefault(param_entry_s *pParam);
const char *GetParamValue(param_entry_s *pParam);
bool FileExists(const char *file);
inline bool FileExists(const std::string &file) { return FileExists(file.c_str()); };
bool WriteAccess(const char *file);
inline bool WriteAccess(const std::string &file) { return WriteAccess(file.c_str()); };

// These functions should be defined for each frontend
void throwerror(bool dialog, const char *error, ...);
void EndProg(void);

#endif 
