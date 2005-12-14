#include "main.h"

#include <stdarg.h>

namespace LIBSU
{

void log(const char *txt, ...)
{
#ifdef ENABLE_LOGGING
    // Ineffcient perhaps, but works for forked processes

    FILE *LogFile = NULL;
    static bool InitLogFile = true;
    static char buffer[1024];
    static va_list v;
    
    if (InitLogFile)
    {
        LogFile = fopen("log.txt", "w"); // Clear file on start
        InitLogFile = false;
    }
    else
        LogFile = fopen("log.txt", "a");
    
    if (!LogFile) return;
    
    va_start(v, txt);
        vsprintf(buffer, txt, v);
    va_end(v);
    
    fprintf(LogFile, "%s\n", buffer);
    fclose(LogFile);
    LogFile = NULL;
#endif
}

bool FileExists(const char *file)
{
    struct stat st;
    return (lstat(file, &st) == 0);
}

}
