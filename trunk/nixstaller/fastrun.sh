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

# Find main nixstaller directory
if [ ! -z `dirname $0` ]; then
    NDIR="`dirname $0`"
    echo "$NDIR" | grep "^/" >/dev/null || NDIR="`pwd`/$NDIR"
else
    for F in $PATH
    do
        if [ -f "$F/$0" ]; then
            NDIR="$F"
            break
        fi
    done
fi

. "$NDIR/src/internal/utils.sh"

usage()
{
    echo "Usage: $0 [options] <project dir>"
    echo
    echo "[options] can be one of the following things (all are optional):"
    echo
    echo " --help, -h                       Print this message."
    echo " --frontend, -f                   Limit to given frontend(s). Multiple frontends must be seperated by whitespace."
    exit 1
}

startfastinst()
{
    NDIR="$1"
    shift
    
    FRS=
    while true
    do
        case "${1}" in
            --frontend | -f )
                if [ "$2" = "ncurses" ]; then
                    FRS="$FRS ncurs" # Convert from user string
                else
                    FRS="$FRS $2"
                fi
                shift 2
                ;;
            -* )
                usage
                ;;
            * )
                break
                ;;
        esac
    done
    
    [ -z "$FRS" ] && FRS="$FRONTENDS"
    
    for F in $FRS; do
        FOUND=
        for AF in $FRONTENDS; do
            if [ $AF = $F ]; then
                FOUND=1
            fi
        done
        if [ -z $FOUND ]; then
            echo "Error: invalid frontend specified. Valid frontends: gtk fltk ncurses"
            exit 1
        fi
    done

    PRDIR="$1"
    
    if [ -z "$PRDIR" ]; then
        usage
    fi
    
    # UNDONE
    launchfrontend "$NDIR" "$FRS" -c "$PRDIR" -l "$NDIR/src/lua" --ls "$NDIR/../shared/lua" --fastrun -n "$NDIR"
}

startfastinst "$NDIR" $@
