#!/bin/sh

# Copyright (C) 2006 - 2008 Rick Helmus (rhelmus_AT_gmail.com)
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


# Use some SH first to bootstrap...

# Find main nixstaller directory
if [ ! -z `dirname $0` ]; then
    NDIR="`dirname $0`"
    echo "$NDIR" | grep "^/" >/dev/null || NDIR="`pwd`/$NDIR"
else
    OLDIFS="$IFS"
    
    for F in $PATH
    do
        if [ -f "$F/$0" ]; then
            NDIR="$F"
            break
        fi
    done
    
    IFS="$OLDIFS"
fi

source "$NDIR/src/internal/utils.sh"

if [ -z "${1}" -o "${1}" = "--help" -o "${1}" = "-h" ]; then
    echo "Usage: $0 <config dir> [ <installer name> ]"
    echo
    echo " <config dir>: The directory which holds the install config files"
    echo " <installer name>: The file name of the created installer. Default: setup.sh"
    exit 1
fi

if [ ! -d "${1}" ]; then
    echo "No such directory: ${1}"
    exit 1
fi

BIN=`getluabin "$NDIR"`

if [ ! -z "$BIN" ]; then
    "$BIN" -c "$NDIR/src/lua/geninstall.lua" $NDIR $@ || exit 1
    exit 0
fi

echo "Could not find a suitable binary for this platform ($CURRENT_ARCH, $CURRENT_OS)"
exit 1
