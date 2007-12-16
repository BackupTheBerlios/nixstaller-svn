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

# Find main nixstaller directory
if [ ! -z `dirname $0` ]; then
    NDIR="`dirname $0`"
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

CURRENTOS=`uname`
CURRENTOS=`echo "$CURRENTOS" | tr [:upper:] [:lower:]` # Convert to lowercase

CURRENT_ARCH=`uname -m`
echo $CURRENT_ARCH | grep "i.86" >/dev/null && CURRENT_ARCH="x86" # Convert iX86 --> x86
echo $CURRENT_ARCH | grep "86pc" >/dev/null && CURRENT_ARCH="x86" # Convert 86pc --> x86

# Default settings
DEF_APPNAME="My App"
DEF_ARCHIVETYPE="lzma"
DEF_DEFAULTLANG="english"
DEF_TARGETOS="$CURRENTOS"
DEF_TARGETARCH="$CURRENT_ARCH"
DEF_FRONTENDS="gtk fltk ncurses"
DEF_LANGUAGES="english dutch"
DEF_INTROPIC=
DEF_LOGO=

# Valid settings
VAL_ARCHIVETYPES="lzma gzip bzip2"
VAL_TARGETOS="freebsd linux netbsd openbsd sunos"
VAL_TARGETARCH="x86 x86_64"
VAL_FRONTENDS="gtk fltk ncurses"

APPNAME=
ARCHIVETYPE=
DEFAULTLANG=
TARGETOS=
TARGETARCH=
FRONTENDS=
LANGUAGES=
INTROPIC=
LOGO=
TARGETDIR=

# List containing languages that couldn't be found in main lang/ directory
NEWLANGUAGES=

usage()
{
    # UNDONE
    echo "Usage: $0 [options] <target dir>"
    echo
    echo "[options] can be one of the following things (all are optional):"
    echo
    echo " --appname, -n name               The application name. Default: My App"
    echo " --arch arch1[, arch2, ...]       CPU architectures the installer should support. Valid values: x86, x86_64. Default: current arch."
    echo " --archtype, -a lzma/gzip/bzip2   The archive type used for packing the installation files. Default: lzma"
    echo " --deflang, -d lang               The default language. Default: english"
    echo " --frontends, -f fr1[, fr2, ...]  Frontends to include. Valid values: gtk, fltk, ncurses. Default: gtk fltk ncurses."
    echo " --intropic, -i picture           Path to picture file, which is displayed in the welcomescreen. Valid types are png, jpeg, gif and bmp. Default: none"
    echo " --languages, -l l1[, l2, ...]    Languages to include (copied from main lang/ directory). Default: english dutch"
    echo " --logo file                      Path to logo picture file. Valid types: png, jpeg, gif and bmp. Default: a default logo."
    echo " --os, -o os1[, os2, ...]         Operating systems which the installer should support. Valid values: linux, freebsd, netbsd, openbsd, sunos. Default: current OS"
    exit 1
}

error()
{
    echo "$*"
    exit 1
}

requiredcp()
{
    cp "$@" || error "Failed to copy file(s)"
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
        case "${1}" in
            --help | -h)
                usage
                ;;
            --appname | -n)
                shift
                [ -z "${1}" ] && usage
                APPNAME="${1}"
                shift
                ;;
            --archtype | -a)
                shift
                [ -z "${1}" ] && usage
                verifyentry "${1}" $VAL_ARCHIVETYPES || error "Wrong archive type specified, valid values are: $VAL_ARCHIVETYPES"
                ARCHIVETYPE="${1}"
                shift
                ;;
            --deflang | -d)
                shift
                [ -z "${1}" ] && usage
                DEFAULTLANG="${1}"
                shift
                ;;
            --os | -o)
                shift
                [ -z "${1}" ] && usage
                while [ $# -gt 1 ] && checkargentry "${1}"
                do
                    verifyentry "${1}" $VAL_TARGETOS || error "Wrong os value specified, valid values are: $VAL_TARGETOS"
                    TARGETOS="$TARGETOS $1"
                    shift
                done
                ;;
            --arch)
                shift
                [ -z "${1}" ] && usage
                while [ $# -gt 1 ] && checkargentry "${1}"
                do
                    verifyentry "${1}" $VAL_TARGETARCH || error "Wrong os value specified, valid values are: $VAL_TARGETARCH"
                    TARGETARCH="$TARGETARCH $1"
                    shift
                done
                ;;
            --frontends | -f)
                shift
                [ -z "${1}" ] && usage
                while [ $# -gt 1 ] && checkargentry "${1}"
                do
                    verifyentry "${1}" $VAL_FRONTENDS || error "Wrong os value specified, valid values are: $VAL_FRONTENDS"
                    FRONTENDS="$FRONTENDS $1"
                    shift
                done
                ;;
            --languages | -l)
                shift
                [ -z "${1}" ] && usage
                while [ $# -gt 1 ] && checkargentry "${1}"
                do
                    LANGUAGES="$LANGUAGES $1"
                    shift
                done
                ;;
            --intropic | -i)
                shift
                [ -z "${1}" ] && usage
                [ ! -f "${1}" ] && error "Couldn't find intro picture ($1)"
                INTROPIC=$1
                shift
                ;;
            --logo)
                shift
                [ -z "${1}" ] && usage
                [ ! -f "${1}" ] && error "Couldn't find logo file ($1)"
                LOGO="${1}"
                shift
                ;;
            -*)
                usage
                ;;
            *)
                break
                ;;
        esac
    done
    
    TARGETDIR="${1}"
    
    if [ -z "${TARGETDIR}" ]; then
        usage
    elif [ -d "${TARGETDIR}" ]; then
        printf "Warning: target directory already exists. Delete it? (y/N) "
        read yn
        if [ "$yn" = "y" -o "$yn" = "Y" ]; then
            echo "Removing $TARGETDIR ..."
            rm -rf "${TARGETDIR}"
        fi
    fi
    
    # Set defaults where necessary
    APPNAME=${APPNAME:=$DEF_APPNAME}
    ARCHIVETYPE=${ARCHIVETYPE:=$DEF_ARCHIVETYPE}
    DEFAULTLANG=${DEFAULTLANG:=$DEF_DEFAULTLANG}
    TARGETOS=${TARGETOS:=$DEF_TARGETOS}
    TARGETARCH=${TARGETARCH:=$DEF_TARGETARCH}
    FRONTENDS=${FRONTENDS:=$DEF_FRONTENDS}
    LANGUAGES=${LANGUAGES:=$DEF_LANGUAGES}
    INTROPIC=${INTROPIC:=$DEF_INTROPIC}
    LOGO=${LOGO:=$DEF_LOGO}
}

createlayout()
{
    mkdir -p "${TARGETDIR}" || error "Failed to create target directory"
    mkdir -p "${TARGETDIR}/files_all" || error "Failed to create files_all directory"
    mkdir -p "${TARGETDIR}/files_extra" || error "Failed to create files_extra directory"
}

copylanguages()
{
    for L in $LANGUAGES
    do
        SRC=
        if [ ! -d "${NDIR}/lang/${L}" ]; then
            NEWLANGUAGES="$NEWLANGUAGES $L"
            SRC="${NDIR}/lang/english/strings"
        else
            SRC="${NDIR}/lang/${L}/strings"
        fi
        
        DEST="${TARGETDIR}"/lang/"${L}"
        mkdir -p "${DEST}"
        requiredcp "${SRC}" "${DEST}"
    done
}

copyintropic()
{
    if [ ! -z "${INTROPIC}" ]; then
        requiredcp "${INTROPIC}" "${TARGETDIR}/files_extra"
    fi
}

copylogo()
{
    if [ ! -z "${LOGO}" ]; then
        requiredcp "${LOGO}" "${TARGETDIR}"
    fi
}

# Converts 'sh lists' to lua tables
toluatable()
{
    RET="{ "
    
    for v in $*
    do
        if [ $v = $1 ]; then
            RET="$RET \"$v\"" # Skip comma on first arg
        else
            RET="$RET, \"$v\""
        fi
    done
    RET="$RET }"

    echo $RET
}

genconfig()
{
    IP=
    if [ ! -z "${INTROPIC}" ]; then
        IP=\"`basename "${INTROPIC}"`\"
    fi
    
    IL=
    if [ ! -z "${LOGO}" ]; then
        IL=\"`basename "${LOGO}"`\"
    fi

    cat > ${TARGETDIR}/config.lua  << EOF
-- Automaticly generated on `date`.
-- Global configuration file, used for generating and running installers.

-- The application name
cfg.appname = "$APPNAME"

-- Archive type used to pack the installer
cfg.archivetype = "$ARCHIVETYPE"

-- Default language (you can use this to change the language of the language
-- selection screen)
cfg.defaultlang = "$DEFAULTLANG"

-- Target Operating Systems
cfg.targetos = `toluatable $TARGETOS`

-- Target CPU Architectures
cfg.targetarch = `toluatable $TARGETARCH`

-- Frontends to include
cfg.frontends = `toluatable $FRONTENDS`

-- Translations to include
cfg.languages = `toluatable $LANGUAGES`

-- Picture used for the 'WelcomeScreen'
cfg.intropic = ${IP:=nil}

-- Logo image file
cfg.logo = ${IL:=nil}
EOF
}

genrun()
{
    cat > ${TARGETDIR}/run.lua  << EOF
-- Automaticly generated on `date`.
-- This file is called when the installer is run.
-- Don't place any (initialization) code outside the Init() or Install() functions.

function Init()
    -- This function is called as soon as the user launches the installer.
    
    -- The destination directory for the installation files. The 'SelectDirScreen' lets the user
    -- change this variable.
    install.destdir = os.getenv("HOME")
    
    -- Installation screens to show (in given order). Custom screens should be placed here.
    install.screenlist = { WelcomeScreen, LicenseScreen, SelectDirScreen, InstallScreen, FinishScreen }
end

function Install()
    -- This function is called as soon as the 'InstallScreen' is shown.
    
    -- This function extracts the files to 'install.destdir'.
    install.extractfiles()
end
EOF
}

createprojdir()
{
    createlayout
    copylanguages
    copyintropic
    copylogo
    genconfig
    genrun
}

hints()
{
    echo "Base project directory layout created in ${TARGETDIR}."
    echo
    echo "In this directory you can..."
    echo "* Edit config.lua for general configuration options. (file has comments)"
    echo "* Edit run.lua for scripting the installation process. (file has comments)"
    echo "* Place the installation files in the files_all/ directory. (for platform specific files see the manual)"
    echo "* Put any files needed in runtime by the installer in the files_extra/ directory."
    echo "* Create 'welcome', 'license' or 'finish' text files for setting the text used by their respective screens."
    echo "* Create new translations in the lang/ directory."
    
    if [ ! -z "$NEWLANGUAGES" ]; then
        echo "NOTE: The following specified languages aren't yet translated: $NEWLANGUAGES."
    fi
}

parseargs "$@"
createprojdir
hints

