#include "main.h"

#include <stdarg.h>

FILE *gpLogFile = NULL;
bool gbInitLogFile = false;

void log(const char *txt, ...)
{
    static char buffer[1024];
    static va_list v;
    
    if (!gbInitLogFile)
    {
        gpLogFile = fopen("log.txt", "w");
        gbInitLogFile = true;
    }
    
    if (!gpLogFile) return;
    
    va_start(v, txt);
        vsprintf(buffer, txt, v);
    va_end(v);
    
    fprintf(gpLogFile, buffer);
    fflush(gpLogFile);
}

// Used for a child process
void rewind_log()
{
    static int nr = 1;
    if (!gbInitLogFile) return;
    
    char fname[32];
    sprintf(fname, "log%d.txt", nr);
    gpLogFile = fopen(fname, "w");
    gbInitLogFile = true;
    nr++;
}

void close_log()
{
    if (gpLogFile) fclose(gpLogFile);
    gpLogFile = NULL;
}

void exit_error(const char *txt, ...)
{
    static char buffer[1024];
    static va_list v;

    va_start(v, txt);
        vsprintf(buffer, txt, v);
    va_end(v);
    
    log(buffer);
    close_log();
    
    exit(1);
}
