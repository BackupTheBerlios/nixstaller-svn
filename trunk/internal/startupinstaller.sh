#!/bin/sh

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
USELIBC="." # Incase no libc specific frontend is found, use the one in the main frontend dir
CURRENT_ARCH=`uname -m`

# iX86 --> x86
echo $CURRENT_ARCH | grep "i*86" >/dev/null && CURRENT_ARCH="x86"

# Find out existing libc's
LIBCS=`ls /lib/libc.so.* | sort -nr`
#echo "Found following LIBC's:"
#echo "${LIBCS}"

# Check if we can run on the users OS
if [ ! -d ./frontends/$CURRENT_OS/$CURRENT_ARCH ]; then
    echo "WARNING: Unsupported OS/Architecture"
    echo "WARNING: Defaulting to Linux/$CURRENT_ARCH"
    CURRENT_OS="linux"
fi

# Still can't run?
if [ ! -d ./frontends/$CURRENT_OS/$CURRENT_ARCH ]; then
    echo "Error: No suitable frontend for \"$OS\"/$CURRENT_ARCH found, aborting"
    exit 1
fi

if test -z ${LIBCS} 2>/dev/null; then
    echo "WARNING: Couldn't detect any existing libc libraries"
    echo "WARNING: Defaulting to libc.so.6"
    LIBCS="/lib/libc.so.6"
fi

# Check which libc to use
for L in $LIBCS
do
    L=`echo $L | sed -e 's/\/lib\///g' -e 's/\.so\.//g'` # Convert /lib/libc.so.X to libcX
    if [ -d "./frontends/${CURRENT_OS}/${CURRENT_ARCH}/${L}" ]; then
        USELIBC=$L
        #echo "Using libc \"${USELIBC}\""
        break
    fi
done

NCURS="none"
FLTK="none"
RUNCOMMAND=

# Check if ncurses frontend exists
if [ -e ./frontends/$CURRENT_OS/${CURRENT_ARCH}/${USELIBC}/ncurs ]; then
    NCURS="./frontends/${CURRENT_OS}/${CURRENT_ARCH}/${USELIBC}/ncurs"
fi


# Check if fltk frontend exists
if [ -e ./frontends/$CURRENT_OS/${CURRENT_ARCH}/${USELIBC}/fltk ]; then
    FLTK="./frontends/${CURRENT_OS}/${CURRENT_ARCH}/${USELIBC}/fltk"
fi


# Do both frontends exist?
if [ $NCURS != "none" -a $FLTK != "none" ]; then
    # X Running?
    if [ -z $DISPLAY ]; then
        RUNCOMMAND="${NCURS}" # Not running, use ncurses frontend
    else
        RUNCOMMAND="${FLTK}"
    fi
elif [ $NCURS != "none" ]; then
    RUNCOMMAND="${NCURS}"
elif [ $FLTK != "none" ]; then
    RUNCOMMAND="${FLTK}"
else
    echo "Error: Couldn't find any frontend to use!"
    exit 1
fi

# And here we go...
$RUNCOMMAND
