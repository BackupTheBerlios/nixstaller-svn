#ifndef MAIN_H
#define MAIN_H

void check(void);
bool MainInit(int argc, char *argv[]);
bool ReadConfig(void);
float ExtractArchive(char *curfile);

enum EArchiveType { ARCH_GZIP, ARCH_BZIP2 };
struct install_info_s
{
    int version;
    char program_name[129];
    char arch_name[2048];
    char dest_dir[2048];
    EArchiveType archive_type;
    
    install_info_s(void) : version(1), archive_type(ARCH_GZIP) { strcpy(program_name, "foo"); };
};

extern install_info_s InstallInfo;

#endif 
