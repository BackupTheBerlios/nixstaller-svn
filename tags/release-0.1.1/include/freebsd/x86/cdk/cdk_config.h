/* include/cdk_config.h.  Generated automatically by configure.  */
/* 
 * $Id: config.hin,v 1.2 2000/01/17 14:48:19 tom Exp $
 */

#ifndef CDK_CONFIG_H
#define CDK_CONFIG_H 1


#define CC_HAS_PROTOS 1
#define CDK_PATCHDATE 20050424
#define CDK_VERSION "5.0"
#define HAVE_CURSES_H 1
#define HAVE_CURSES_H 1
#define HAVE_DIRENT_H 1
#define HAVE_GETBEGX 1
#define HAVE_GETBEGY 1
#define HAVE_GETCWD 1
#define HAVE_GETMAXX 1
#define HAVE_GETMAXY 1
#define HAVE_GETOPT_H 1
#define HAVE_GETOPT_HEADER 1
#define HAVE_LIMITS_H 1
#define HAVE_LSTAT 1
#define HAVE_MKTIME 1
#define HAVE_SETLOCALE 1
#define HAVE_START_COLOR 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_TERM_H 1
#define HAVE_TYPE_CHTYPE 1
#define HAVE_UNISTD_H 1
#define MIXEDCASE_FILENAMES 1
#define NCURSES 1
#define PACKAGE "cdk"
#define STDC_HEADERS 1
#define SYSTEM_NAME "freebsd6.0"
#define TYPE_CHTYPE_IS_SCALAR 1
#define setbegyx(win,y,x) ((win)->_begy = (y), (win)->_begx = (x), OK)

#if !defined(HAVE_LSTAT) && !defined(lstat)
#define lstat(f,b) stat(f,b)
#endif

#endif /* CDK_CONFIG_H */
