#include "main.h"

#include <stdarg.h>

void log(const char *txt, ...)
{
    // Ineffcient perhaps, but works for forked processes

    FILE *LogFile = NULL;
    static bool InitLogFile = false;
    static char buffer[1024];
    static va_list v;
    
    if (InitLogFile)
    {
        LogFile = fopen("log.txt", "w"); // Clear file on start
        InitLogFile = true;
    }
    else
        LogFile = fopen("log.txt", "a");
    
    if (!LogFile) return;
    
    va_start(v, txt);
        vsprintf(buffer, txt, v);
    va_end(v);
    
    fprintf(LogFile, buffer);
    fclose(LogFile);
    LogFile = NULL;
}

void exit_error(const char *txt, ...)
{
    static char buffer[1024];
    static va_list v;

    va_start(v, txt);
        vsprintf(buffer, txt, v);
    va_end(v);
    
    log(buffer);

    exit(1);
}

bool FileExists(const char *file)
{
    FILE *f = fopen(file, "r");
    if (f)
    {
        fclose(f);
        return true;
    }

    return false;
}
