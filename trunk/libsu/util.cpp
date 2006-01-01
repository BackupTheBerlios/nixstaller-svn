/*
    Copyright (C) 2006 by Rick Helmus (rhelmus@gmail.com)

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
