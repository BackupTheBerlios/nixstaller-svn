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
    if [ ! -z "$1" -a -f $1.lzma ]; then
        $2/lzma-decode $1.lzma $1 2>&1 >/dev/null && rm $1.lzma
    fi
}

getlibs()
{
    LIB=$1
    RET=""
    shift
    
    for D in $*; do
        DIR=`echo "$D/$LIB"* | sort -nr`
        if [ "$DIR" != "$D/$LIB*" ]; then # Did evaluate?
            RET="$RET $DIR"
        fi
    done
    
    echo $RET
    unset LIB RET DIR
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

    # Nexenta (and Solaris) have 2 terminfo directories: one in
    # /usr/share/terminfo and one in /usr/share/lib/terminfo. Solaris ncurses
    # works with both, Nexenta only with the second on a regular terminal.
    # Another catch is that Solaris (but not Nexenta...) doesn't work with the
    # terminal 'sun-color' but does with 'sun'. So we overide the 2 here...
    # UNDONE
    
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

    # Get all C libs. Sorted so higher versions come first
    LIBCS=`getlibs libc.so. /lib /usr/lib`

    echo "C libraries: $LIBCS"
    
    # Get all C++ libs. Sorted so higher versions come first
    LIBSTDCPPS=`getlibs libstdc++.so. /usr/lib /usr/sfw/lib /usr/lib/libstdc++-v3`
    
    echo "C++ libraries: $LIBSTDCPPS"
}

# Uses edelta to reconstruct frontend binaries
# $1: libc directory to be used(for lzma and edelta)
# $2: source file
# $3: diff file
edelta()
{
    unlzma $3 $1
    
    mv $3 $3.tmp
    $1/edelta -q patch $2 $3 $3.tmp >/dev/null
}

# Gets valid bin dir
# $1: Full libname (ie /lib/libc.so.5)
# $2: Unversioned short libname (ie libc)
# $3: Path where directory resides
getbinlibdir()
{
    BASE=`basename $1`
    MAJOR=`echo $BASE | awk 'BEGIN { FS="." } /.so./ { for (i=1; i<=NF; i++) if ($i == "so") print $(i+1) }'`
    MINOR=`echo $BASE | awk 'BEGIN { FS="." } /.so./ { for (i=1; i<=NF; i++) if ($i == "so") print $(i+2) }'`

    if [ ! -z "$MINOR" ]; then
        echo ${3}/$2.so.$MAJOR.* | sort -nr | awk '{ print $1 }'
    else
        echo "${3}/$BASE"
    fi

    # Remove unwanted globals
    unset BASE MAJOR MINOR
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

FRONTENDS="fltk ncurs"

for FR in $FRONTENDS
do
    if [ -z "$DISPLAY" -a $FR != "ncurs" ]; then
        continue
    fi
    
    case $FR in
        "fltk") ED_SRC=$FLTK_SRC ;;
        "ncurs") ED_SRC=$NCURS_SRC ;;
    esac
    
    for LC in $LIBCS
    do
        LCDIR="`getbinlibdir ${LC} libc bin/$CURRENT_OS/$CURRENT_ARCH`"
        
        if [ ! -d "${LCDIR}" ]; then
            continue
        fi
    
        if [ $ARCH_TYPE = "lzma" -a ! -f ${LCDIR}/lzma-decode ]; then
            continue # No usable lzma-decoder
        fi
        
        unlzma $ED_SRC ${LCDIR}
        
        for LCPP in $LIBSTDCPPS
        do
            LCPPDIR="`getbinlibdir ${LCPP} libstdc++ ${LCDIR}`"
            
            if [ ! -d ${LCPPDIR} ]; then
                continue
            fi
            
            unlzma ${LCPPDIR}/$FR ${LCDIR}
            
            if [ -f "${LCPPDIR}/$FR" ]; then
                FRBIN="${LCPPDIR}/$FR"
                if [ ! -z "$ED_SRC" -a $FRBIN != $ED_SRC ]; then
                    edelta ${LCDIR} $ED_SRC $FRBIN
                fi
                
                # Run it
                chmod +x $FRBIN # deltas en lzma packed bins probably aren't executable yet
                `pwd`/$FRBIN
                exit $?
            fi
        done
    done
done

echo "Error: Couldn't find any suitable frontend for your system"
exit 1
