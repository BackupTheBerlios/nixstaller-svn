#!/bin/sh
# 
#     Copyright (C) 2006, 2007 Rick Helmus (rhelmus_AT_gmail.com)
# 
#     This file is part of Nixstaller.
#     
#     This program is free software; you can redistribute it and/or modify it under
#     the terms of the GNU General Public License as published by the Free Software
#     Foundation; either version 2 of the License, or (at your option) any later
#     version. 
#     
#     This program is distributed in the hope that it will be useful, but WITHOUT ANY
#     WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
#     PARTICULAR PURPOSE. See the GNU General Public License for more details. 
#     
#     You should have received a copy of the GNU General Public License along with
#     this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
#     St, Fifth Floor, Boston, MA 02110-1301 USA

CURDIR=`pwd`

CURRENTOS=`uname`
CURRENTOS=`echo "$CURRENTOS" | tr [:upper:] [:lower:]` # Convert to lowercase

# Default settings
DEF_TARGETOS="$CURRENTOS"
DEF_TARGETARCH="x86"
DEF_FRONTENDS="gtk fltk ncurses"
DEF_LANGUAGES="english dutch"
DEF_INTROPIC="nil"

# Valid settings
VAL_TARGETOS="freebsd linux netbsd openbsd sunos"
VAL_TARGETARCH="x86"
VAL_FRONTENDS="gtk fltk ncurses"

TARGETOS=
TARGETARCH=
FRONTENDS=
LANGUAGES=
INTROPIC=
TARGETDIR=

# List containing languages that couldn't be found in main lang/ directory
NEWLANGUAGES=

usage()
{
    echo "Usage: $0 [options] <target dir>"
    echo
    echo "options can be one of the following things (all optional):"
    echo
    echo " --os, -o os1[, os2, ...]: Operating systems which the installer should support. Valid values: linux, freebsd, netbsd, openbsd, sunos. Default: current OS"
    echo " --arch arch1[, arch2, ...]: CPU architectures the installer should support. Valid value: x86. Default: x86"
    exit 1
}

error()
{
    echo "$*"
    exit 1
}

requiredcp()
{
    cp $* || error "Failed to copy file(s)"
}

checkargentry()
{
    if [ -z "$1" ]; then
        return 1
    fi
    
    case "$1" in
        -*)
            return 1
            ;;
        *)
            return 0
            ;;
    esac
}

# $1: Value to check
# $*: Valid values
verifyentry()
{
    VAL="$1"
    shift
    for o in $*
    do
        if [ $o = $VAL ]; then
            return 0
        fi
    done
    
    return 1
}

parseargs()
{
    while true
    do
        case $1 in
            --help | -h)
                usage
                ;;
            --os | -o)
                shift
                [ -z $1 ] && usage
                while [ $# -gt 1 ] && checkargentry $1
                do
                    verifyentry $1 $VAL_TARGETOS || error "Wrong os value specified, valid values are: $VAL_TARGETOS"
                    TARGETOS="$TARGETOS $1"
                    shift
                done
                ;;
            --arch | -a)
                shift
                [ -z $1 ] && usage
                while [ $# -gt 1 ] && checkargentry $1
                do
                    verifyentry $1 $VAL_TARGETARCH || error "Wrong os value specified, valid values are: $VAL_TARGETARCH"
                    TARGETARCH="$TARGETARCH $1"
                    shift
                done
                ;;
            --frontends | -f)
                shift
                [ -z $1 ] && usage
                while [ $# -gt 1 ] && checkargentry $1
                do
                    verifyentry $1 $VAL_FRONTENDS || error "Wrong os value specified, valid values are: $VAL_FRONTENDS"
                    FRONTENDS="$FRONTENDS $1"
                    shift
                done
                ;;
            --languages | -l)
                shift
                [ -z $1 ] && usage
                while [ $# -gt 1 ] && checkargentry $1
                do
                    LANGUAGES="$LANGUAGES $1"
                    shift
                done
                ;;
            --intropic | -i)
                shift
                [ -z $1 ] && usage
                # UNDONE
                ;;

            *)
                break
                ;;
        esac
    done
    
    TARGETDIR="$1"
    
    if [ -z "${TARGETDIR}" ]; then
        usage
    elif [ -e "${TARGETDIR}" ]; then
        printf "Warning: target directory already exists. Delete it? (y/N) "
        read yn
        if [ "$yn" = "y" -o "$yn" = "Y" ]; then
            echo "Removing $TARGETDIR ..."
            rm -rf ${TARGETDIR}
        fi
    fi
    
    # Set defaults where necessary
    TARGETOS=${TARGETOS:=$DEF_TARGETOS}
    TARGETARCH=${TARGETARCH:=$DEF_TARGETARCH}
    FRONTENDS=${FRONTENDS:=$DEF_FRONTENDS}
    LANGUAGES=${LANGUAGES:=$DEF_LANGUAGES}
    INTROPIC=${INTROPIC:=$DEF_INTROPIC}
}

copylanguages()
{
    for L in $LANGUAGES
    do
        SRC=
        if [ ! -e ${CURDIR}/lang/${L} ]; then
            NEWLANGUAGES="$NEWLANGUAGES $L"
            SRC=${CURDIR}/lang/english/strings
        else
            SRC=${CURDIR}/lang/${L}/strings
        fi
        
        DEST=${TARGETDIR}/lang/${L}
        mkdir -p ${DEST}
        requiredcp ${SRC} ${DEST}
    done
}

# Converts 'sh lists' to lua tables
toluatable()
{
    RET="{ "
    
    for v in $*
    do
        if [ $v = $1 ]; then
            RET="$RET $v" # Skip comma on first arg
        else
            RET="$RET, $v"
        fi
    done
    RET="$RET }"

    echo $RET
}

genconfig()
{
    cat > ${TARGETDIR}/config.lua  << EOF
-- Automaticly generated on `date`
config.targetos = `toluatable $TARGETOS`
config.targetarch = `toluatable $TARGETARCH`
config.frontends = `toluatable $FRONTENDS`
config.languages = `toluatable $LANGUAGES`
config.intropic = $INTROPIC
EOF

}

createprojdir()
{
    mkdir -p $TARGETDIR || error "Failed to create target directory"
    
    copylanguages
    genconfig
    
    # Generate install.lua
    # Copy intropic
}

parseargs $*
createprojdir
# Print hints
