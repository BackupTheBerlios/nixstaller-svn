#!/bin/sh

# Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)
# 
# This file is part of Nixstaller.
# 
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version. 
# 
# This program is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE. See the GNU General Public License for more details. 
# 
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
# St, Fifth Floor, Boston, MA 02110-1301 USA

# Does some simple checking, and tries to launch the right installer frontend

. "./utils.sh"

UNATTENDED=

# $1: Base directory
# $2: Base lib name
createliblist()
{
    echo `find "$1/$2"* 2>/dev/null | sort -nr`
}

checksys()
{
    if [ ! -d "./bin/$CURRENT_OS/" -a "$CURRENT_OS" != "linux" ]; then
        echo "Warning: No binaries for \"$CURRENT_OS\" found, trying to default to Linux..."
        CURRENT_OS="linux"
    fi

    if [ ! -d "./bin/$CURRENT_OS/$CURRENT_ARCH/" -a "$CURRENT_ARCH" != "x86" ]; then
        echo "Warning: No binaries for \"$CURRENT_ARCH\" found, trying to default to x86..."
        CURRENT_ARCH="x86"
    fi
}

configure()
{
    echo "Collecting info for this system..."
    
    checksys
    
    echo "Operating system: $CURRENT_OS"   
    echo "CPU Arch: $CURRENT_ARCH"
    
    while [ "$1" != "" ]
    do
        case "${1}" in
            --frontend )
                FRONTENDS="$2"
                [ "$FRONTENDS" = "ncurses" ] && FRONTENDS="ncurs" # Convert from user string
                shift 2
                ;;
            --unattended )
                UNATTENDED="unattended"
                shift
                ;;
            *)
                ARGS="$ARGS $1"
                shift
                ;;
        esac
    done
}

# Taken from makeself.sh
progress()
{
    while read a; do
        printf .
    done
}

extractsub()
{
    printf "Uncompressing sub archive"
    if [ $ARCH_TYPE = "gzip" ]; then
        cat subarch | gzip -cd | tar xvf - 2>&1 | progress
    elif [ $ARCH_TYPE = "bzip2" ]; then
        cat subarch | bzip2 -d | tar xvf - 2>&1 | progress
    else # lzma
        checksys
        
        # Find usable lzma-decode binary
        
        LZMA_DECODE=
        for LC in `createliblist bin/$CURRENT_OS/$CURRENT_ARCH libc.so.`
        do
            if [ ! -d "${LC}" ]; then
                continue
            fi
            
            if [ -f "${LC}/lzma-decode" ]; then
                if haslibs "${LC}/lzma-decode" ; then
                    LZMA_DECODE="${LC}/lzma-decode"
                    break
                fi
            fi
        done
        
        if [ -z "$LZMA_DECODE" ]; then
            # Static binary?
            if [ -f "bin/$CURRENT_OS/$CURRENT_ARCH/lzma-decode" ]; then
                LZMA_DECODE="bin/$CURRENT_OS/$CURRENT_ARCH/lzma-decode"
            fi
        fi
        
        if [ -z $LZMA_DECODE ]; then
            echo "Error: No lzma decoder found for $CURRENT_OS/$CURRENT_ARCH"
            exit 1
        fi
        
        $LZMA_DECODE subarch - 2>/dev/null | tar xvf - 2>&1 | progress
    fi
    
    echo
}

launchfrontend()
{
    DIR="$1"
    STATIC=$2
    
    for FR in $FRONTENDS
    do
        if [ -z "$DISPLAY" -a $FR != "ncurs" -a -z "$UNATTENDED" ]; then
            continue
        fi
        
        if [ -f "${DIR}/$FR" ]; then
            FRBIN="${DIR}/$FR"
            
            # Incase it isn't executable yet...
            # NOTE: This needs to be before the 'haslibs' call, since ldd wants an executable file
            chmod +x $FRBIN
            
            if [ -z "$STATIC" ]; then
                haslibs $FRBIN || continue
            fi
            
            # Run it
            if [ $FR = "ncurs" ] && [ $CURRENT_OS = "freebsd" -o $CURRENT_OS = "netbsd" -o $CURRENT_OS = "openbsd" ]; then
                # Try to launch ncurses frontend with supplied terminfo's
                export TERMINFO="`pwd`/terminfo"
            fi
            
            `pwd`/$FRBIN $UNATTENDED -l "`pwd`" --ls "`pwd`/sharedmain" -c "`pwd`/config" -- $ARGS
            
            RET=$?
            if [ ! -f frontendstarted ]; then
                exit $RET
            fi
        fi
    done
}


# Check which archive type to use
ARCH_TYPE=`cat info`
if [ -z "$ARCH_TYPE" ]; then
    ARCH_TYPE="lzma"
fi

extractsub
configure $*

touch frontendstarted # Frontend removes this file if it's started

for LC in `createliblist bin/$CURRENT_OS/$CURRENT_ARCH libc.so.`
do
    if [ ! -d "${LC}" ]; then
        continue
    fi
    
    launchfrontend "$LC"
done

# If nothing found, check for any static bins (ie. openbsd ncurses)
DIR="bin/$CURRENT_OS/$CURRENT_ARCH"
[ $CURRENT_OS = "openbsd" ] && echo $FRONTENDS | grep -e "fltk" -e "gtk" >/dev/null && echo "NOTE: Graphical frontends are only supported for OpenBSD 4.4."
launchfrontend "$DIR" 1


echo "Error: Couldn't find any suitable frontend for your system"
exit 1
