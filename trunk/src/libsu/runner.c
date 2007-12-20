/*
    Copyright (C) 2006, 2007 by Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of libsu.

    This file contains code of the KDE project, module kdesu.
    Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>

    This file contains code from TEShell.C of the KDE konsole.
    Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de> 

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

/* Program executed by libsu. This program will execute another program. */

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

int main(int argc, char **argv)
{
    pid_t pid, ppid = getppid();
    int retcode = EXIT_SUCCESS;

    if (argc < 2)
        return EXIT_FAILURE;
    
    pid = fork();
    if (pid == 0) // Child
    {
        if (setsid() == -1)
            _exit(127);
        
        execlp("/bin/sh", "sh", "-c", argv[1], NULL);
        _exit(127);
    }
    
    while (ppid == getppid())
    {
        int state, ret = waitpid(pid, &state, WNOHANG);
        struct timespec tsec = { 0, 150000000 };
        
        if (ret < 0)
            return EXIT_FAILURE;
        
        if (ret == pid)
        {
            if (WIFEXITED(state))
                retcode = WEXITSTATUS(state);
            break;
        }
        
        nanosleep(&tsec, NULL);
    }
    
    kill(-pid, SIGTERM);
    
    return retcode;
}
