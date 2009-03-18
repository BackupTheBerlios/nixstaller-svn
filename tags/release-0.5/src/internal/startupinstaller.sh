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

# Uses xdelta3 to reconstruct frontend binaries
# $1: Binary path
# $2: source file
# $3: diff file
xdelta3()
{
    unlzma "$3" "$1"
    
    mv "$3" "$3.tmp"
    "$1/xdelta3" -dfs "$2" "$3.tmp" "$3" >/dev/null
}

# Unpacks $1.lzma to lzma and removes $1.lzma
# $1: File to handle
# $2: Binary path
unlzma()
{
    if [ $ARCH_TYPE = "lzma" -a ! -z "$1" -a -f "$1".lzma ]; then
        "${2}/lzma-decode" "${1}.lzma" "$1" 2>&1 >/dev/null && rm "${1}.lzma"
    fi
}

# Prepares frontend binary (extracting and patching)
# $1: Binary path
# $2: frontend
prepbin()
{
    BIN="$1/$2"
    PATCHORDER="$BIN"
    TOPBIN=
    
    while [ ! -z "$BIN" ]
    do
        BASE=`awk '$1=="'"$BIN"'"{print $2}' ./bin/deltas`
        
        if [ -z "$BASE" ]; then
            # Top bin
            TOPBIN="$BIN"
            BIN=
        else
            BIN="$BASE"
            PATCHORDER="$BASE $PATCHORDER"
        fi
    done
    
    PREV=
    for BIN in $PATCHORDER
    do
        if [ ! -f "$BIN" ]; then # Not prepared yet?
            if [ "$BIN" = "$TOPBIN" ]; then
                unlzma "$BIN" "$1"
            else
                unlzma "$BIN.diff" "$1"
                xdelta3 "$1" "$PREV" "$BIN.diff"
                mv "$BIN.diff" "$BIN"
            fi
        fi
        PREV="$BIN"
    done
}

# $1: Base directory
# $2: Base lib name
createliblist()
{
    echo `find "$1/$2"* 2>/dev/null | sort -nr`
}

configure()
{
    echo "Collecting info for this system..."
    
    echo "Operating system: $CURRENT_OS"
    
    if [ ! -d "./bin/$CURRENT_OS/" ]; then
        echo "Warning: No installer for \"$CURRENT_OS\" found, defaulting to Linux..."
        CURRENT_OS="linux"
    fi

    echo "CPU Arch: $CURRENT_ARCH"

    if [ ! -d "./bin/$CURRENT_OS/$CURRENT_ARCH/" ]; then
        echo "Warning: No installer for \"$CURRENT_ARCH\" found, defaulting to x86..."
        CURRENT_ARCH="x86"
    fi
    
    while [ "$1" != "" ]
    do
        case "${1}" in
            --frontend )
                FRONTENDS="$2"
                [ "$FRONTENDS" = "ncurses" ] && FRONTENDS="ncurs" # Convert from user string
                shift 2
                ;;
            --unattended )
                ARGS="unattended"
                UNATTENDED=1
                shift
                ;;
            *)
                ARGS="$ARGS $1"
                shift
                ;;
        esac
    done
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
        
        if [ -z "$STATIC" ]; then
            [ $ARCH_TYPE != "lzma" ] || haslibs "${DIR}/lzma-decode" || continue
            haslibs "${DIR}/xdelta3" || continue
        fi

        prepbin "$DIR" "$FR"
        
        if [ -f "${DIR}/$FR" ]; then
            FRBIN="${DIR}/$FR"
            
            # deltas and lzma packed bins probably aren't executable yet
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
            
            `pwd`/$FRBIN $ARGS
            
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
    ARCH_TYPE="gzip"
fi

configure $*

touch frontendstarted # Frontend removes this file if it's started

for LC in `createliblist bin/$CURRENT_OS/$CURRENT_ARCH libc.so.`
do
    if [ ! -d "${LC}" ]; then
        continue
    fi

    if [ $ARCH_TYPE = "lzma" -a ! -f "${LC}/lzma-decode" ]; then
        continue # No usable lzma-decoder
    fi
    
    launchfrontend "$LC"
done

# If nothing found, check for any static bins (ie. openbsd ncurses)
DIR="bin/$CURRENT_OS/$CURRENT_ARCH"
if [ $ARCH_TYPE != "lzma" -o -f "${DIR}/lzma-decode" ]; then
    [ $CURRENT_OS = "openbsd" ] && echo $FRONTENDS | grep -e "fltk" -e "gtk" >/dev/null && echo "NOTE: Graphical frontends are only supported for OpenBSD 4.4."
    launchfrontend "$DIR" 1
fi


echo "Error: Couldn't find any suitable frontend for your system"
exit 1
