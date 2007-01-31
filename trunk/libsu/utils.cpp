/*
    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of libsu.

    libsu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    libsu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libsu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

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
    char *buffer;
//     static char fname[512];
    const char *fname = "/tmp/libsu.log";
    static va_list v;
    
    if (InitLogFile)
    {
/*        getcwd(fname, sizeof(fname));
        strncat(fname, "/../lsulog.txt", sizeof(fname)-strlen("/../lsulog.txt"));*/
        LogFile = fopen(fname, "w"); // Clear file on start
        InitLogFile = false;
    }
    else
        LogFile = fopen(fname, "a");
    
    if (!LogFile)
        return;
    
    va_start(v, txt);
    vasprintf(&buffer, txt, v);
    va_end(v);
    
    fprintf(LogFile, "%s\n", buffer);
    fclose(LogFile);
    free(buffer);
#endif
}

bool FileExists(const char *file)
{
    struct stat st;
    return (lstat(file, &st) == 0);
}

char *FormatText(const char *str, ...)
{
    char *ret;
    va_list v;
    
    va_start(v, str);
    vasprintf(&ret, str, v);
    va_end(v);
    
    return ret;
}

}
