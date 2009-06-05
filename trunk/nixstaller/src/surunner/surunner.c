/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

    This file is part of Nixstaller.
    
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version. 
    
    This program is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE. See the GNU General Public License for more details. 
    
    You should have received a copy of the GNU General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
    St, Fifth Floor, Boston, MA 02110-1301 USA
*/

/*
Program executed by CSuTerm. This program will simply execute another program.
This makes it easy to manage it and get it's return status
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

int main(int argc, char **argv)
{
    pid_t pid, ppid;
    int retcode = EXIT_SUCCESS;

    if (argc < 3)
        return EXIT_FAILURE;

    ppid = atoi(argv[1]); /*The real caller (ie. the installer itself, not a fork)*/
    pid = fork();

    if (pid == -1)
        return EXIT_FAILURE;
    
    if (pid == 0) // Child
    {
        if (setsid() == -1)
            _exit(127);
        
        execlp("/bin/sh", "sh", "-c", argv[2], NULL);
        _exit(127);
    }
    
    while (!kill(ppid, 0))
    {
        int state, ret = waitpid(pid, &state, WNOHANG);
        struct timespec tsec = { 0, 150000000 };
        
        if (ret < 0)
        {
            retcode = EXIT_FAILURE;
            break;
        }
        
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
