#!/bin/sh
# 
#     Copyright (C) 2006 - 2009 Rick Helmus (rhelmus_AT_gmail.com)
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
if [ ! -z "`dirname $0`" ]; then
    NDIR="`dirname $0`"
else
    for F in $PATH
    do
        if [ -f "$F/$0" ]; then
            NDIR="$F"
            break
        fi
    done
fi

# Default settings
DEF_MODE="attended"
DEF_APPNAME="My App"
DEF_ARCHIVETYPE="lzma"
DEF_DEFAULTLANG="english"
DEF_TARGETOS=
DEF_TARGETARCH=
DEF_FRONTENDS="gtk fltk ncurses"
DEF_LANGUAGES="english dutch"
DEF_INTROPIC=
DEF_LOGO=
DEF_APPICON=
DEF_AUTOLANG="true"

# Valid settings
VAL_MODE="both attended unattended"
VAL_ARCHIVETYPES="lzma gzip bzip2"
VAL_TARGETOS="freebsd linux netbsd openbsd sunos"
VAL_TARGETARCH="x86 x86_64"
VAL_FRONTENDS="gtk fltk ncurses"

MODE=
APPNAME=
ARCHIVETYPE=
DEFAULTLANG=
TARGETOS=
TARGETARCH=
FRONTENDS=
LANGUAGES=
AUTOLANG=
INTROPIC=
LOGO=
APPICON=
TARGETDIR=
GENPKG=
OVERWRITE=
RMEXISTING=
DEPS=

# List containing languages that couldn't be found in main lang/ directory
NEWLANGUAGES=

usage()
{
    echo "Usage: $0 [options] <target dir>"
    echo
    echo "[options] can be one of the following things (all are optional):"
    echo
    echo " --help, -h                       Print this message."
    echo " --appicon <file>                 Path to appicon file. This should be a XPM file. Default: a default icon."
    echo " --appname, -n <name>             The application name. Default: My App"
    echo " --arch <arch>                    One or more CPU architectures the installer should support. Valid values: x86, x86_64. Default: current arch."
    echo " --archtype, -a lzma/gzip/bzip2   The archive type used for packing the installation files. Default: lzma"
    echo " --deflang, -d <lang>             The default language. Default: english"
    echo " --deps                           Add specific dependency code to generated lua scripts. Needs --pkg option."
    echo " --frontends, -f <frontends>      One or more frontends to include. Valid values: gtk, fltk and ncurses. Default: gtk fltk ncurses."
    echo " --intropic, -i <picture>         Path to picture file, which is displayed in the welcomescreen. Valid types are png, jpeg, gif and bmp. Default: none"
    echo " --languages, -l <langs>          Languages to include (copied from main lang/ directory). Default: english dutch"
    echo " --logo <file>                    Path to logo picture file. Valid types: png, jpeg, gif and bmp. Default: a default logo."
    echo " --mode, -m <mode>                 Sets the installer mode. Valid values: both, attended, unattended. Default: attended"
    echo " --no-autolang                     Disables automaticly choosing a language."
    echo " --os, -o <OSs>                   Operating systems which the installer should support. Valid values: linux, freebsd, netbsd, openbsd, sunos. Default: current OS"
    echo " --overwrite                      Overwrite any existing files. Default is to ask."
    echo " --pkg                            Generate a template package.lua with defaults, adds specific 'Package Mode' code to install.lua."
    echo " --rm-existing                     Removes any existing files. Default is to ask."
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
            --appicon)
                shift
                [ -z "${1}" ] && usage
                [ ! -f "${1}" ] && error "Couldn't find appicon file ($1)"
                APPICON="${1}"
                shift
                ;;
            --pkg)
                shift
                GENPKG=1
                ;;
            --overwrite)
                shift
                OVERWRITE=1
                ;;
            --rm-existing)
                shift
                RMEXISTING=1
                ;;
            --deps)
                shift
                DEPS=1
                ;;
            --mode | -m)
                shift
                [ -z "${1}" ] && usage
                verifyentry "${1}" $VAL_MODE || error "Wrong archive type specified, valid values are: $VAL_MODE"
                MODE="${1}"
                shift
                ;;
            --no-autolang)
                AUTOLANG="false"
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
        if [ "$RMEXISTING" = 1 ]; then
            rm -rf "${TARGETDIR}"
        elif [ -z "$OVERWRITE" ]; then
            printf "Warning: target directory already exists. Delete it? (y/N) "
            read yn
            if [ "$yn" = "y" -o "$yn" = "Y" ]; then
                echo "Removing $TARGETDIR ..."
                rm -rf "${TARGETDIR}"
            fi
        fi
    fi
    
    # Set defaults where necessary
    MODE=${MODE:=$DEF_MODE}
    APPNAME=${APPNAME:=$DEF_APPNAME}
    ARCHIVETYPE=${ARCHIVETYPE:=$DEF_ARCHIVETYPE}
    DEFAULTLANG=${DEFAULTLANG:=$DEF_DEFAULTLANG}
    AUTOLANG=${AUTOLANG:=$DEF_AUTOLANG}
    TARGETOS=${TARGETOS:=$DEF_TARGETOS}
    TARGETARCH=${TARGETARCH:=$DEF_TARGETARCH}
    FRONTENDS=${FRONTENDS:=$DEF_FRONTENDS}
    LANGUAGES=${LANGUAGES:=$DEF_LANGUAGES}
    INTROPIC=${INTROPIC:=$DEF_INTROPIC}
    LOGO=${LOGO:=$DEF_LOGO}
    APPICON=${APPICON:=$DEF_APPICON}
}

createlayout()
{
    mkdir -p "${TARGETDIR}" || error "Failed to create target directory"
    mkdir -p "${TARGETDIR}/files_all" || error "Failed to create files_all directory"
    mkdir -p "${TARGETDIR}/files_extra" || error "Failed to create files_extra directory"

    if [ ! -z "$TARGETOS" ]; then
        # OS Specific file dirs
        for OS in $TARGETOS
        do
            mkdir -p "${TARGETDIR}/files_${OS}_all" || error "Failed to create files_$OS_all directory."

            if [ ! -z "$TARGETARCH" ]; then
                # OS/Arch specific dirs
                for ARCH in $TARGETARCH
                do
                    mkdir -p "${TARGETDIR}/files_${OS}_${ARCH}" || error "Failed to create files_$OS_$ARCH directory."
                done
            fi
        done
    fi

    if [ ! -z "$TARGETARCH" ]; then
        # Arch specific dirs
        for ARCH in $TARGETARCH
        do
            mkdir -p "${TARGETDIR}/files_all_${ARCH}" || error "Failed to create files_all_$ARCH directory."
        done
    fi
}

copylanguages()
{
    for L in $LANGUAGES
    do
        SRC=
        if [ ! -d "${NDIR}/lang/${L}" ]; then
            NEWLANGUAGES="$NEWLANGUAGES $L"
            SRC="${NDIR}/lang/english/"
        else
            SRC="${NDIR}/lang/${L}/"
        fi
        
        DEST="${TARGETDIR}"/lang/"${L}"
        mkdir -p "${DEST}"
        requiredcp "${SRC}/strings" "${DEST}"
        cp "${SRC}/config.lua" "${DEST}"
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
        requiredcp "${LOGO}" "${TARGETDIR}/files_extra"
    fi
}

copyappicon()
{
    if [ ! -z "${APPICON}" ]; then
        requiredcp "${APPICON}" "${TARGETDIR}/files_extra"
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

getruninit()
{
    if [ "$MODE" != "unattended" ]; then
        if [ ! -z "$GENPKG" ]; then
            cat << EOF
function Init()
    -- This function is called as soon as an attended (default) installation is started.
    
    -- The files that need to be packaged should be placed inside the directory returned from
    -- 'install.getpkgdir()'. By setting install.destdir to this path, the packed installation files
    -- are directly extracted, (re)packaged and installed to the user's system. If you need to compile
    -- the software on the user's system, you probably need to extract the files somewhere else and place
    -- any compiled files inside the 'temporary package directory' later.
    install.destdir = install.getpkgdir()
    
    -- Installation screens to show (in given order). Custom screens should be placed here.
    install.screenlist = { WelcomeScreen, LicenseScreen, PackageToggleScreen, PackageDirScreen, InstallScreen, SummaryScreen, FinishScreen }
end
EOF
        else
            cat << EOF
function Init()
    -- This function is called as soon as an attended (default) installation is started.
    
    -- The destination directory for the installation files. The 'SelectDirScreen' lets the user
    -- change this variable.
    install.destdir = os.getenv("HOME")
    
    -- Installation screens to show (in given order). Custom screens should be placed here.
    install.screenlist = { WelcomeScreen, LicenseScreen, SelectDirScreen, InstallScreen, FinishScreen }
end
EOF
        fi
    fi
}

getrununinit()
{
    if [ "$MODE" != "attended" ]; then
        if [ ! -z "$GENPKG" ]; then
            cat << EOF
function UnattendedInit()
    -- This function is called as soon as an unattended installation is started.
    
    -- The files that need to be packaged should be placed inside the directory returned from
    -- 'install.getpkgdir()'. By setting install.destdir to this path, the packed installation files
    -- are directly extracted, (re)packaged and installed to the user's system. If you need to compile
    -- the software on the user's system, you probably need to extract the files somewhere else and place
    -- any compiled files inside the 'temporary package directory' later.
    install.destdir = install.getpkgdir()
end
EOF
        else
            cat << EOF
function UnattendedInit()
    -- This function is called as soon as an unattended installation is started.
    
    -- Set install.destdir to a default incase the user did not gave then --destdir
    -- option (remove this if cfg.adddestunopt() is not called in config.lua!).
    if not cfg.unopts["destdir"].value then
        install.destdir = os.getenv("HOME") -- Default directory
    end
end
EOF
        fi
        echo
    fi
}

getruninstall()
{
    if [ ! -z "$GENPKG" ]; then
        cat << EOF
function Install()
    -- This function is called as soon as the installation starts (for attended installs,
    -- when the 'InstallScreen' is shown).
    
    -- How many 'steps' the installation has (used to divide the progressbar). Since
    -- install.extractfiles, pkg.verifydeps and install.generatepkg all 'use' one step, we start with 3.
    install.setstepcount(3)
    
    -- Check if we need root access. By asking the user here, he or she can decide to proceed
    -- or not before the actual installation begins.
    if pkg.needroot() then
        install.askrootpw()
    end
    
    -- This function extracts the files to 'install.destdir'.
    install.extractfiles()
    
    -- Verifies, downloads and merges any dependencies to the temporary package directory.
    pkg.verifydeps()
    
    -- This function generates and installs the package.
    -- If you need to call 'install.gendesktopentries', do this before this function.
    install.generatepkg()
end
EOF
    else
        cat << EOF
function Install()
    -- This function is called as soon as the installation starts (for attended installs,
    -- when the 'InstallScreen' is shown).
    
    -- This function extracts the files to 'install.destdir'.
    install.extractfiles()
end
EOF
    fi
}

getfinishinstall()
{
    cat << EOF
function Finish(err)
    -- This function is called when the installer exits. The variable 'err' is a bool,
    -- that is true when an error occured. Common usages for this function are clearing temporary
    -- files and executing a final command (when err is false).
end
EOF
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

    TOS=
    if [ ! -z "${TARGETOS}" ]; then
        TOS="`toluatable $TARGETOS`"
    fi

    TARCH=
    if [ ! -z "${TARGETARCH}" ]; then
        TARCH="`toluatable $TARGETARCH`"
    fi
    
    APP=
    if [ ! -z "${APPICON}" ]; then
        APP=\"`basename "${APPICON}"`\"
    fi

    cat > ${TARGETDIR}/config.lua  << EOF
-- Automaticly generated on `date`.
-- Global configuration file, used for generating and running installers.

-- Installer mode. Valid values: "both", "attended" (default) and "unattended".
cfg.mode = "$MODE"

-- The application name
cfg.appname = "$APPNAME"

-- Archive type used to pack the installer
cfg.archivetype = "$ARCHIVETYPE"

-- Target Operating Systems
cfg.targetos = ${TOS:=nil}

-- Target CPU Architectures
cfg.targetarch = ${TARCH:=nil}

-- Frontends to include
cfg.frontends = `toluatable $FRONTENDS`

-- Default language (you can use this to change the language of the language
-- selection screen)
cfg.defaultlang = "$DEFAULTLANG"

-- Translations to include
cfg.languages = `toluatable $LANGUAGES`

-- When enabled the installer will automaticly guess the right language. If this fails
-- or when this option is disabled, the user has to choose a language.
cfg.autolang = $AUTOLANG

-- Picture used for the 'WelcomeScreen'
cfg.intropic = ${IP:=nil}

-- Logo image file
cfg.logo = ${IL:=nil}

-- Appication icon used by the installer
cfg.appicon = ${APP:=nil}
EOF

    if [ "$MODE" != "attended" ]; then
        cat >> ${TARGETDIR}/config.lua  << EOF

-- Commandline options for unattended installations

-- Uncomment the next line to add license options. This will also make sure
-- that the user has to accept the license.
-- cfg.addlicenseunopts()
EOF
        if [ -z "$GENPKG" ]; then
            cat >> ${TARGETDIR}/config.lua  << EOF

-- Option to set install.destdir
cfg.adddestunopt()
EOF
        fi
    fi
}

genrun()
{
    cat > ${TARGETDIR}/run.lua  << EOF
-- Automaticly generated on `date`.
-- This file is called when the installer is run.
-- Don't place any (initialization) code outside the Init(), UnattendedInit() or Install() functions.

EOF
    code="`getruninit`"
    [ ! -z "$code" ] && printf "$code\n\n" >> "${TARGETDIR}/run.lua"
    
    code="`getrununinit`"
    [ ! -z "$code" ] && printf "$code\n\n" >> "${TARGETDIR}/run.lua"

    code="`getruninstall`"
    [ ! -z "$code" ] && printf "$code\n\n" >> "${TARGETDIR}/run.lua"
    
    code="`getfinishinstall`"
    [ ! -z "$code" ] && printf "$code\n\n" >> "${TARGETDIR}/run.lua"
}

genpkg()
{
    cat > ${TARGETDIR}/package.lua  << EOF
-- Automaticly generated on `date`.
-- This file is used for 'Package Mode' configuration.

-- Enables or disables Package Mode
pkg.enable = true

-- The package's name. Usually a simple (no spaces etc.) and lowercased
-- version of 'cfg.appname'
pkg.name = "mypkg"

-- The software version
pkg.version = "1"

-- The package version (see the manual why this is different than pkg.version).
pkg.release = "1"

-- A short (max 1 line) description.
pkg.summary = "Change this summary"

-- A longer description for the installed package. Can be a few lines long
pkg.description = [[
Change this longer description.
Which can have multiple lines.]]

-- A group to which this package belongs to. See the manual for valid groups.
pkg.group = "File" -- You probably want to change this!

-- The software license of this package
pkg.license = "GPLv2"

-- The package maintainer/creator. Usually your name + email
pkg.maintainer = "$USER $USER@email.com"

-- URL to software's homepage
pkg.url = "www.google.com"

-- Table (array) that should contain (relative!) paths to any binaries that
-- should have a 'binary script' (usually each binary).
-- This field is also used for pkg.verifydeps() (if no arguments are given) or
-- when pkg.autosymmap is true.
pkg.bins = {}

-- Table (array) that should contain (relative!) paths to any 'main libraries'
-- (libraries not coming from dependency packages). This field is only used by
-- (pkg.verifydeps() or when pkg.autosymmap is true.
pkg.libs = {}

-- Table (array) containing all dependency packages for the 'main dependencies'
-- (so direct dependencies and not dependencies for other dependencies). The strings
-- used in this table should be the same as the dependency package directory name.
pkg.deps = {}

-- (Default) Directory used as 'data directory'. When unspecified (or nil) a
-- reasonable default is automaticly choosen.
pkg.destdir = nil

-- (Default) Directory used to place any 'binary scripts'. When unspecified (or nil)
-- a reasonable default is automaticly choosen.
pkg.bindir = nil

-- Enable this for KDE software
pkg.setkdeenv = false

-- Whether the installer should (try to) register the software with the user's package
-- manager.
pkg.register = true

-- If enabled, the installer will try to automaticly generate a symmap.lua. It's
-- assumed that all binaries/libraries are in the project's and dependency package's
-- files_<os>_<arch>/ directory (where <os> is the current OS and <arch> the current
-- CPU architecture).
pkg.autosymmap = false
EOF

    if [ "$MODE" != "attended" ]; then
        cat >> ${TARGETDIR}/package.lua  << EOF

-- Adds common commandline options for unattended installations. If you don't want any
-- dependency specific options pass false as first argument.
pkg.addpkgunopts(true)
EOF
    fi
}

createprojdir()
{
    createlayout
    copylanguages
    copyintropic
    copylogo
    copyappicon
    genconfig
    genrun
    
    if [ ! -z "$GENPKG" ]; then
        genpkg
    fi
}

hints()
{
    echo "Base project directory layout created in ${TARGETDIR}."
    echo
    echo "In this directory you can..."
    echo "* Edit config.lua for general configuration options. (file has comments)"
    echo "* Edit run.lua for scripting the installation process. (file has comments)"
    
    if [ ! -z "$GENPKG" ]; then
        echo "* Edit package.lua for package mode configuration. (file has comments)"
        echo "* Generate package dependencies. Use deputils.sh to make this easier."
        echo "* Generate a symmap.lua file with gensyms.sh or do this automaticly by enabling pkg.symmap. This file is used for binary compatibility checking."
    fi
    
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

