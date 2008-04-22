/*
    Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of libsu.

    This file contains code of the KDE project, module kdesu.
    Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>

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

// This project is heavily based on 'kdesu'

#ifndef LSU_MAIN_H
#define LSU_MAIN_H

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/resource.h>

#include "config.h"
#include "libsu.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE   /* Needed for getpt, ptsname in glibc 2.1.x systems */
#endif

#if defined(__osf__)
#include <pty.h>
#endif

// stdlib.h is meant to declare the prototypes but doesn't :(
#ifndef __THROW
#define __THROW
#endif

#ifdef HAVE_GRANTPT
extern "C" int grantpt(int fd) __THROW;
#endif

#ifdef HAVE_PTSNAME
extern "C" char * ptsname(int fd) __THROW;
#endif

#ifdef HAVE_UNLOCKPT
extern "C" int unlockpt(int fd) __THROW;
#endif

#ifdef HAVE_GETPTY
extern "C" char *_getpty(int *, int, mode_t, int);
#endif

#ifndef HAVE_VASPRINTF
extern "C" int vasprintf (char **result, const char *format, va_list args);
#endif

#ifdef HAVE_PTY_H
    #include <pty.h>
#endif

#include <termios.h>

#ifdef HAVE_LIBUTIL_H
    #include <libutil.h>
#elif defined(HAVE_UTIL_H)
    #include <util.h>
#endif

#if defined(__SVR4) && defined(sun)
#include <stropts.h>
#include <sys/stream.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>                // Needed on some systems.
#endif


namespace LIBSU
{

/*
 * Steven Schultz <sms at to.gd-es.com> tells us :
 * BSD/OS 4.2 doesn't have a prototype for openpty in its system header files
 */
#ifdef __bsdi__
__BEGIN_DECLS
int openpty(int *, int *, char *, struct termios *, struct winsize *);
__END_DECLS
#endif

void log(const char *txt, ...);
bool FileExists(const char *file);
inline bool FileExists(const std::string &file) { return FileExists(file.c_str()); };
char *FormatText(const char *str, ...);
}

#endif
