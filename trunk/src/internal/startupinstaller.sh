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

# Uses edelta to reconstruct frontend binaries
# $1: libc directory to be used(for lzma and edelta)
# $2: source file
# $3: diff file
edelta()
{
    unlzma "$3" "$1"
    
    mv "$3" "$3.tmp"
    "$1/edelta" -q patch "$2" "$3" "$3.tmp" >/dev/null
}

# Unpacks $1.lzma to lzma and removes $1.lzma
# $1: File to handle
# $2: libc directory to be used for lzma
unlzma()
{
    if [ $ARCH_TYPE = "lzma" -a ! -z "$1" -a -f "$1".lzma ]; then
        "${2}/lzma-decode" "${1}.lzma" "$1" 2>&1 >/dev/null && rm "${1}.lzma"
    fi
}

# Prepares frontend binary (extracting and patching)
# $1: libc directory for used binaries
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
                edelta "$1" "$PREV" "$BIN.diff"
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
    
    for FR in $FRONTENDS
    do
        if [ -z "$DISPLAY" -a $FR != "ncurs" -a -z "$UNATTENDED" ]; then
            continue
        fi
        
        echo "Frontend: $FR"
    
        [ $ARCH_TYPE != "lzma" ] || haslibs "${LC}/lzma-decode" || continue
        haslibs "${LC}/edelta" || continue

        prepbin "$LC" "$FR"
        
        if [ -f "${LC}/$FR" ]; then
            FRBIN="${LC}/$FR"
            
            # deltas and lzma packed bins probably aren't executable yet
            # NOTE: This needs to be before the 'haslibs' call, since ldd wants an executable file
            chmod +x $FRBIN
            
            haslibs $FRBIN || continue
            
            # Run it
            `pwd`/$FRBIN $ARGS
            
            RET=$?
            if [ ! -f frontendstarted ]; then
                exit $RET
            fi
        fi
    done
done

echo "Error: Couldn't find any suitable frontend for your system"
exit 1
