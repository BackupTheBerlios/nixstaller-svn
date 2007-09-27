#!/bin/sh

# Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)
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

# Unpacks $1.lzma to lzma and removes $1.lzma
# $1: File to handle
# $2: libc directory to be used for lzma
unlzma()
{
    if [ $ARCH_TYPE = "lzma" -a ! -z "$1" -a -f $1.lzma ]; then
        "${2}/lzma-decode" "${1}.lzma" $1 2>&1 >/dev/null && rm "${1}.lzma"
    fi
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
    
    OS=`uname`
    CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`
    echo "Operating system: $CURRENT_OS"
    
    if [ ! -d "./bin/$CURRENT_OS/" ]; then
        echo "Warning: No installer for \"$CURRENT_OS\" found, defaulting to Linux..."
        CURRENT_OS="linux"
    fi

    CURRENT_ARCH=`uname -m`
    # iX86 --> x86
    echo $CURRENT_ARCH | grep "i.86" >/dev/null && CURRENT_ARCH="x86"

    # Solaris uses i86pc instead of x86 (or i*86), convert
    echo $CURRENT_ARCH | grep "i86pc" >/dev/null && CURRENT_ARCH="x86"   

    echo "CPU Arch: $CURRENT_ARCH"

    if [ ! -d "./bin/$CURRENT_OS/$CURRENT_ARCH/" ]; then
        echo "Warning: No installer for \"$CURRENT_ARCH\" found, defaulting to x86..."
        CURRENT_ARCH="x86"
    fi
}

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

# $1: Executable that ldd needs to research
haslibs()
{
    if [ $CURRENT_OS = "openbsd" ]; then
        (ldd "$1" >/dev/null 2>&1) && return 0
    else
        if [ -z "`ldd \"$1\" | grep 'not found'`" ]; then
           return 0
        fi
    fi
    return 1
}

# Check which archive type to use
ARCH_TYPE=`cat info`
if [ -z "$ARCH_TYPE" ]; then
    ARCH_TYPE="gzip"
fi

configure

# Get source frontend to use for edelta
NCURS_SRC=`awk '$1=="ncurs"{print $2}' ./bin/$CURRENT_OS/$CURRENT_ARCH/edelta_src`
FLTK_SRC=`awk '$1=="fltk"{print $2}' ./bin/$CURRENT_OS/$CURRENT_ARCH/edelta_src`
GTK_SRC=`awk '$1=="gtk"{print $2}' ./bin/$CURRENT_OS/$CURRENT_ARCH/edelta_src`

FRONTENDS="gtk fltk ncurs"

touch frontendstarted # Frontend removes this file if it's started

for LC in `createliblist bin/$CURRENT_OS/$CURRENT_ARCH libc.so.`
do
    if [ ! -d "${LC}" ]; then
        continue
    fi

    if [ $ARCH_TYPE = "lzma" -a ! -f "${LC}/lzma-decode" ]; then
        continue # No usable lzma-decoder
    fi
    
    for LCPP in `createliblist "${LC}" libstdc++.so.`
    do
        if [ ! -d "${LCPP}" ]; then
            continue
        fi
              
        for FR in $FRONTENDS
        do
            if [ -z "$DISPLAY" -a $FR != "ncurs" ]; then
                continue
            fi
        
            case $FR in
                "gtk") ED_SRC=$GTK_SRC ;;
                "fltk") ED_SRC=$FLTK_SRC ;;
                "ncurs") ED_SRC=$NCURS_SRC ;;
            esac
            
            [ $ARCH_TYPE != "lzma" ] || haslibs "${LC}/lzma-decode" || continue
            haslibs "${LC}/edelta" || continue

            unlzma $ED_SRC "${LC}"
            unlzma "${LCPP}"/$FR "${LC}"
            
            if [ -f "${LCPP}/$FR" ]; then
                FRBIN="${LCPP}/$FR"
                if [ ! -z "$ED_SRC" -a $FRBIN != $ED_SRC ]; then
                    edelta ${LC} $ED_SRC $FRBIN
                fi
                
                # deltas and lzma packed bins probably aren't executable yet
                # NOTE: This needs to be before the 'haslibs' call, since ldd wants an executable file
                chmod +x $FRBIN
                
                haslibs $FRBIN || continue
                
                # Run it
                `pwd`/$FRBIN
                
                RET=$?
                if [ ! -f frontendstarted ]; then
                    exit $RET
                fi
            fi
        done
    done
done

echo "Error: Couldn't find any suitable frontend for your system"
exit 1
