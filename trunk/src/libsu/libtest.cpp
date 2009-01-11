/*
    Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)

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

#include "libsu.h"

using namespace LIBSU;

void printout(const char *s, void *p) { printf(s); }

int main()
{
    char command[128], user[128], passwd[128];
    printf("user: ");
    scanf("%s", user);
    printf("password: ");
    if (scanf("%s", passwd)) // Everyone can see your password...not good
    {
        CLibSU LibSU("ls", user);
        LibSU.SetPath("/bin:/usr/bin:/usr/pkg/bin:/usr/local/bin");
        LibSU.SetTerminalOutput(false);
        LibSU.SetOutputFunc(printout);
        printf("ExecuteCommand: %d\n", LibSU.ExecuteCommand(passwd, true));
    }
    else
        printf("\nSorry\n");
    return 0;
}
