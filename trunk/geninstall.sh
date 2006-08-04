#!/bin/sh

#    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)

#    This file is part of Nixstaller.

#    Nixstaller is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.

#    Nixstaller is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with Nixstaller; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#    Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
#    conditions of the GNU General Public License cover the whole combination.

#    In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
#    software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
#    DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
#    such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
#    you include the source code of that other code when and as the GNU GPL requires distribution of source code.

#    Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
#    versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
#    version without this exception; this exception also makes it possible to release a modified version which carries forward
#    this exception.

ARCHNAME_BASE="instarchive"

# Main functions
# --------------------------------------------------
 
init()
{
    if [ -z $1 ]; then
        echo "Usage: $0 <config dir> [ <installer name> ]"
        echo
        echo " <config dir>: The directory which holds the install config files"
        echo " <installer name>: The file name of the created installer. Default: setup.sh"
        exit 1
    fi
    
    if [ ! -d "${1}" ]; then
        echo "No such directory: ${CONFDIR}"
        exit 1
    fi

    CURDIR=$PWD

    CONFDIR="${1}"
    CONFDIR=${CONFDIR%*/} # If target dir has trailing '/', remove it
    # If target dir doesn't start with '/' insert the current dir to it
    CONFDIR=`echo ${CONFDIR} | grep '^/' || echo ${PWD}/$CONFDIR`

    if [ -z "${2}" ]; then
        OUTNAME="setup.sh"
    else
        OUTNAME="${2}"
    fi
    
    
    local OS=`uname`
    CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]` # Convert to lowercase
    
    CURRENT_ARCH=`uname -m`
    echo $CURRENT_ARCH | grep "i*86" >/dev/null && CURRENT_ARCH="x86" # Convert iX86 --> x86
    echo $CURRENT_ARCH | grep "86pc" >/dev/null && CURRENT_ARCH="x86" # Convert 86pc --> x86
    
    BIN_LZMA=
    
    for LC in `ls -d ${CURDIR}/bin/${CURRENT_OS}/${CURRENT_ARCH}/libc* 2>/dev/null | sort -nr`
    do
        local BIN=${LC}/lzma
        
        if [ -e $BIN ]; then
            if [ ! `ldd  $BIN | grep "not found"` ]; then
                BIN_LZMA=$BIN
                break
            fi
        fi
    done
    
    if [ -z $BIN_LZMA ]; then
        err "Couldn't find a suitable LZMA encoder"
    fi
}

readconfig()
{
    # Check which target frontends there are
    FRONTENDS=`awk '$1=="frontends"{for (i=2;i <= NF;i++) printf("%s ", $i) }' ${CONFDIR}/install.cfg`
    if [ -z "$FRONTENDS" ]; then
        FRONTENDS="ncurses fltk" # Change if there are other frontends
    fi
    
    # Check which target OS'es there are
    TARGET_OS=`awk '$1=="targetos"{for (i=2;i <= NF;i++) printf("%s ", $i) }' ${CONFDIR}/install.cfg`
    if [ -z "$TARGET_OS" ]; then
        TARGET_OS=$CURRENT_OS
    fi
    
    # Check which target archs there are
    TARGET_ARCH=`awk '$1=="targetarch"{for (i=2;i <= NF;i++) printf("%s ", $i) }' ${CONFDIR}/install.cfg`
    if [ -z "$TARGET_ARCH" ]; then
        TARGET_ARCH=$CURRENT_ARCH
    fi
    
    # Check which archive type to use
    ARCH_TYPE=`awk '$1=="archtype"{print $2}' ${CONFDIR}/install.cfg`
    if [ -z "$ARCH_TYPE" ]; then
        ARCH_TYPE="gzip"
    fi
    
    # Check which languages to use
    LANGUAGES=`awk '$1=="languages"{for (i=2;i <= NF;i++) printf("%s ", $i) }' ${CONFDIR}/install.cfg`
    if [ -z "$LANGUAGES" ]; then
        LANGUAGES="english"
    fi
    
    INTROPIC=`awk '$1=="intropic"{print $2}' ${CONFDIR}/install.cfg`

    echo
    echo "Configuration:"
    echo "---------------------------------"
    echo "Installer name: $OUTNAME"
    echo "            OS: $TARGET_OS"
    echo "         Archs: $TARGET_ARCH"
    echo "  Archive type: $ARCH_TYPE"
    echo "     Languages: $LANGUAGES"
    echo "    Config dir: ${CONFDIR}"
    echo "     Frontends: $FRONTENDS"
    echo "     Intro pic: $INTROPIC"
    echo "---------------------------------"
    echo
    echo
}

# This function will copy all files that should be in the final installer archive to a temporary directory
preparearchive()
{
    mkdir -p ${CONFDIR}/tmp/config/
    cp ${CONFDIR}/welcome ${CONFDIR}/tmp/config/ 2>/dev/null
    cp ${CONFDIR}/install.cfg ${CONFDIR}/tmp/config/ || err "Error: no install config file"
    cp ${CONFDIR}/license ${CONFDIR}/tmp/config/ 2>/dev/null
    cp ${CONFDIR}/finish ${CONFDIR}/tmp/config/ 2>/dev/null
    
    cp ${CURDIR}/internal/startupinstaller.sh ${CONFDIR}/tmp || err "Error: missing startupinstaller.sh script"
    cp ${CURDIR}/internal/about ${CONFDIR}/tmp || err "Error: missing about file"
    
    # Copy language files
    for L in $LANGUAGES
    do
        mkdir -p ${CONFDIR}/tmp/config/lang/$L/
        cp ${CONFDIR}/lang/$L/strings ${CONFDIR}/tmp/config/lang/$L/ 2>/dev/null
        cp ${CONFDIR}/lang/$L/welcome ${CONFDIR}/tmp/config/lang/$L/ 2>/dev/null
        cp ${CONFDIR}/lang/$L/license ${CONFDIR}/tmp/config/lang/$L/ 2>/dev/null
        cp ${CONFDIR}/lang/$L/finish ${CONFDIR}/tmp/config/lang/$L/ 2>/dev/null
    done
 
    # Copy OS specific files
    for OS in $TARGET_OS
    do
        [ -e ${CURDIR}/bin/$OS ] || err "Error: no binaries found for $OS"
        
        for ARCH in $TARGET_ARCH
        do
            cd ${CURDIR}/bin/$OS/$ARCH || err "Error: no binaries found for $OS/$ARCH"
            
            for FR in $FRONTENDS
            do
                local FRFOUND=0
                local FRNAME=
                
                case $FR in
                    ncurses )
                        FRNAME="ncurs"
                        ;;
                    fltk )
                        FRNAME="fltk"
                        ;;
                    * )
                        echo "Warning: Unknown frontend specified (\'$FR\')."
                        ;;
                esac
    
                if [ -z $FRNAME ]; then
                    continue
                fi
                
                for LC in `ls -d libc* 2>/dev/null`
                do
                    cd $LC
                    
                    # Copy frontends
                    for LCPP in `ls -d 'libstdc++'* 2>/dev/null`
                    do
                        cd $LCPP
                        local DIR=${CONFDIR}/tmp/bin/$OS/$ARCH/$LC/$LCPP
                        mkdir -p ${DIR}
                        if [ $ARCH_TYPE = "lzma" ]; then
                            $BIN_LZMA e $FRNAME ${DIR}/$FRNAME 2>&1 >/dev/null && FRFOUND=1
                        else
                            cp $FRNAME ${DIR}/ && FRFOUND=1
                        fi
                        cd ..
                    done
                    
                    if [ $ARCH_TYPE = "lzma" ]; then
                        cp lzma-decode ${CONFDIR}/tmp/bin/$OS/$ARCH/$LC/ || err "Error: no lzma decoder for ${OS}/${ARCH}/${LC}"
                    fi
                    cd ..
                done
                
                if [ $FRFOUND -eq 0 ]; then
                    echo "Warning: no $FR frontend for $OS/$ARCH"
                fi
            done
        done
    done

    if [ ! -z ${INTROPIC} ]; then
        cp ${CONFDIR}/${INTROPIC} ${CONFDIR}/tmp || echo "Warning: ${CONFDIR}/$INTROPIC does not exist"
    fi
    
    # Copy all plist files
    PLIST_FOUND=0
    cp ${CONFDIR}/plist_extrpath* ${CONFDIR}/tmp 2>/dev/null && PLIST_FOUND=1
    cp ${CONFDIR}/plist_var_* ${CONFDIR}/tmp 2>/dev/null && PLIST_FOUND=1
    
    if [ $PLIST_FOUND -eq 0 ]; then
        echo "Warning: No plist files found!"
    fi
}

makearchive()
{
    echo "Generating archive..."
    
    # Pack platform independent files
    if [ -d ${CONFDIR}/files_all ]; then
        packdir ${CONFDIR}/files_all "${CONFDIR}/tmp/${ARCHNAME_BASE}_all"
    fi
    
    # Pack arch dependent files
    for ARCH in $TARGET_ARCH
    do
        if [ -d ${CONFDIR}/files_all_${ARCH} ]; then
            packdir ${CONFDIR}/files_all_${ARCH} ${CONFDIR}/tmp/${ARCHNAME_BASE}_all_${ARCH}
        fi
    done
        
    # Pack OS/arch dependent files
    for OS in $TARGET_OS
    do
        if [ -d ${CONFDIR}/files_${OS}_all ]; then
            packdir ${CONFDIR}/files_${OS}_all/ ${CONFDIR}/tmp/${ARCHNAME_BASE}_${OS}
        fi
    
        for ARCH in $TARGET_ARCH
        do
            if [ -d ${CONFDIR}/files_${OS}_${ARCH} ]; then
                packdir ${CONFDIR}/files_${OS}_${ARCH} ${CONFDIR}/tmp/${ARCHNAME_BASE}_${OS}_${ARCH}
            fi
        done
    done
}

createinstaller()
{
    echo "Generating installer..."
    #${CURDIR}/makeself.sh --$ARCH_TYPE ${CONFDIR}/tmp ${CURDIR}/${OUTNAME} "nixstaller" sh ./startupinstaller.sh > /dev/null 2>&1
    ${CURDIR}/makeself.sh --gzip ${CONFDIR}/tmp ${CURDIR}/${OUTNAME} "nixstaller" sh ./startupinstaller.sh > /dev/null 2>&1
}

# Util functions
# --------------------------------------------------

err()
{
    echo $1
    cleanup
    exit 1
}

packdir()
{
    local PKCMD=
    cd $1
    case $ARCH_TYPE in
        gzip )
            # Safe way to pack all files in the current directory, without including the current('.') dir
            tar cf - --exclude .. --exclude . * .?* | gzip -c9 > ${2}
            # Use awk to be able to use files with spaces and omit directory names
            gzip -cd ${2} | tar tf - | awk '{if (system(sprintf("test -d \"%s\"", $0))) printf("\"%s\"\n", $0) | "xargs du"}' > "${2}.sizes"
            ;;
        bzip2 )
            tar cf - --exclude .. --exclude . * .?*  | bzip2 -9 > ${2}
            bzip2 -cd ${2} | tar tf - | awk '{if (system(sprintf("test -d \"%s\"", $0))) printf("\"%s\"\n", $0) | "xargs du"}' > "${2}.sizes"
            ;;
        lzma )
            tar cf ${2}.tmp --exclude .. --exclude . * .?*
            $BIN_LZMA e ${2}.tmp ${2}
            tar tf ${2}.tmp | awk '{if (system(sprintf("test -d \"%s\"", $0))) printf("\"%s\"\n", $0) | "xargs du"}' > "${2}.sizes"
            rm ${2}.tmp
            ;;
        * )
            echo "Error: wrong archive type($ARCH_TYPE). Should be gzip or bzip2"
            exit 1
            ;;
    esac
    [ -f $2 ] || err "Couldn't pack files(archname: $2 dir: $1)"
    cd - # Go back to previous directory
}

cleanup()
{
    # Clean up all temporarily files
    rm -rf ${CONFDIR}/tmp
}

# Main part starts here
# --------------------------------------------------

init "$*"
readconfig
preparearchive
makearchive
createinstaller
cleanup
