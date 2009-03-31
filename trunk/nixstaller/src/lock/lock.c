/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)
    Copyright (c) 2001-2003 Alfredo K. Kojima
    
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

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// Based on code from Synaptic (gsynaptic.cc / TestLock)
pid_t TestLock(const char *file)
{
    int FD = open(file, O_RDONLY);
    if(FD < 0)
    {
        if(errno == ENOENT)
        {
            // File does not exist, no there can't be a lock
            return 0; 
        }
        else
        {
            return -1;
        }
    }
    
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    
    if (fcntl(FD, F_GETLK, &fl) < 0)
    {
        int Tmp = errno;
        close(FD);
        errno = Tmp;
        return -1;
    }
    
    close(FD);
    
    // lock is available
    if(fl.l_type == F_UNLCK)
        return 0;
        
    // file is locked by another process
    return (fl.l_pid);
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    
    int i;
    for (i=1; i<=argc; i++)
    {
        if (TestLock(argv[i-1]) > 0)
            return 0;
    }
    
    return 1;
}
