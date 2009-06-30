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

# Shared SH file

FRONTENDS="gtk fltk ncurs"

OS=`uname`
CURRENT_OS=`echo "$OS" | tr '[A-Z]' '[a-z]'` # Convert to lowercase

CURRENT_ARCH=`uname -m`
echo $CURRENT_ARCH | grep "i.86" >/dev/null && CURRENT_ARCH="x86" # Convert iX86 --> x86
echo $CURRENT_ARCH | grep "86pc" >/dev/null && CURRENT_ARCH="x86" # Convert 86pc --> x86
echo $CURRENT_ARCH | grep "amd64" >/dev/null && CURRENT_ARCH="x86_64" # Convert amd64 --> x86_64

haslibs()
{
    if [ $CURRENT_OS = "openbsd" ]; then
        (ldd "$1" >/dev/null 2>&1) && return 0
    else
        if [ -z "`ldd $1 | grep 'not found'`" ]; then
           return 0
        fi
    fi
    return 1
}

getluabin()
{
    for LC in `ls -d "$1/bin/${CURRENT_OS}/${CURRENT_ARCH}/libc"* 2>/dev/null | sort -nr`
    do
        for F in $FRONTENDS
        do
            BIN="${LC}/$F"
            if [ -f "$BIN" ]; then
                haslibs "$BIN" || continue
                echo "$BIN"
                return 0
            fi
        done
    done
    
    # Check for static bins
    for F in $FRONTENDS
    do
        BIN="$1/bin/${CURRENT_OS}/${CURRENT_ARCH}/$F"
        if [ -f "$BIN" ]; then
            echo "$BIN"
            return 0
        fi
    done
    
    return 1
}

runluascript()
{
    NDIR="$1"
    BIN=`getluabin "$1"`
    SCRIPT="$2"
    shift 2

    if [ ! -z "$BIN" ]; then
        # UNDONE
        "$BIN" run -e "$SCRIPT" -n "$NDIR" -c $0 -l "$NDIR/src/lua" --ls "$NDIR/../shared/lua" -- "$@" || exit 1
        exit 0
    fi
    
    echo "Could not find a suitable binary for this platform ($CURRENT_ARCH, $CURRENT_OS)"
    exit 1
}

startfastinst()
{
    # UNDONE: Make frontend optional
    NDIR="$1"
    BIN=`getluabin "$1"`
    PRDIR="$2"
    shift 2

    if [ ! -z "$BIN" ]; then
        # UNDONE
        "$BIN" -c "$PRDIR" -l "$NDIR/src/lua" --ls "$NDIR/../shared/lua" --fastrun -n "$NDIR" || exit 1
        exit 0
    fi
    
    echo "Could not find a suitable binary for this platform ($CURRENT_ARCH, $CURRENT_OS)"
    exit 1
}
