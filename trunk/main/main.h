#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <list>
#include <map>

enum EArchiveType { ARCH_GZIP, ARCH_BZIP2 };

struct install_info_s
{
    int version;
    char program_name[129];
    char arch_name[2048];
    char dest_dir[2048];
    std::list<std::string> languages;
    std::string cur_lang;
    std::map<std::string, char *> translations;
    EArchiveType archive_type;
    
    install_info_s(void) : version(1), archive_type(ARCH_GZIP) { strcpy(program_name, "foo"); };
};

extern install_info_s InstallInfo;

void check(void);
bool MainInit(int argc, char *argv[]);
void MainEnd(void);
float ExtractArchive(char *curfile);
bool ReadLang(void);
std::string GetTranslation(std::string &s);
char *GetTranslation(char *s);
inline char *GetTranslation(const char *s) { return GetTranslation(const_cast<char *>(s)); };

#endif 
