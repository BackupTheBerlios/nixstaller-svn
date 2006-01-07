#/bin/sh

#    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

#    This file is part of Nixstaller.

#    Nixstaller is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.

#    Nixstaller is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with Nixstaller; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#    Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
#    conditions of the GNU General Public License cover the whole combination.

#    In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
#    software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
#    DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
#    such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
#    you include the source code of that other code when and as the GNU GPL requires distribution of source code.

#    Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
#    versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
#    version without this exception; this exception also makes it possible to release a modified version which carries forward
#    this exception.


# Does some simple checking, and tries to launch the right installer frontend

OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`

# Check if we can run on the users OS
if [ ! -d ./frontends/$CURRENT_OS ]; then
    echo "WARNING: Unsupported OS"
    echo "WARNING: Defaulting to Linux"
    CURRENT_OS="linux"
fi

# Still can't run?
if [ ! -d ./frontends/$CURRENT_OS ]; then
    echo "Error: No suitable frontend for OS \"$OS\" found, aborting"
    exit 1
fi

NCURS="none"
FLTK="none"
RUNCOMMAND=

# Check if ncurses frontend exists
if [ -e ./frontends/$CURRENT_OS/ncurs ]; then
    NCURS="./frontends/${CURRENT_OS}/ncurs"
fi

# Check if fltk frontend exists
if [ -e ./frontends/$CURRENT_OS/fltk ]; then
    FLTK="./frontends/${CURRENT_OS}/fltk"
fi

# Do both frontends exist?
if [ $NCURS != "none" -a $FLTK != "none" ]; then
    # X Running?
    if [ -z $DISPLAY ]; then
        RUNCOMMAND="${NCURS} $CURRENT_OS" # Not running, use ncurses frontend
    else
        RUNCOMMAND="$FLTK $CURRENT_OS"
    fi
elif [ $NCURS != "none" ]; then
    RUNCOMMAND="${NCURS} $CURRENT_OS"
elif [ $FLTK != "none" ]; then
    RUNCOMMAND="$FLTK $CURRENT_OS"
else
    echo "Error: Couldn't find any frontend to use!"
    exit 1
fi

#Incase there are missing libraries, we just change the lib path...
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:./lib/$CURRENT_OS" $RUNCOMMAND
