// This project is heavily based on 'kdelibsu'

#ifndef LSU_MAIN_H
#define LSU_MAIN_H

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
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

#ifdef HAVE__GETPTY
extern "C" char *_getpty(int *, int, mode_t, int);
#endif

#ifdef HAVE__PTY_H
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
}

#endif
