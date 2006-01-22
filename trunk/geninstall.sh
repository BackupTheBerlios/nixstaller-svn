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

ARGS="$*"
CURDIR=$PWD
ARCHNAME_BASE="instarchive"
OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`
CURRENT_ARCH=`uname -m`
CONFDIR="$CURDIR/$1"
TARGET_OS=
TARGET_ARCH=
FRONTENDS=

err()
{
    echo $1
    remtemp
    exit 1
}

checkargs()
{
    if [ -z $ARGS ]; then
        echo "Usage: $0 <config dir>"
        echo
        echo " <config dir>: The directory which holds the install config files"
        exit 1
    fi
    
    if [ ! -d "$CONFDIR" ]; then
        echo "No such directory: $CONFDIR"
        exit 1
    fi
}

copytemp()
{
    # Temporary copy frontends, libs and install config files to install directory
    cp $CONFDIR/welcome $CONFDIR/tmp/config/ 2>/dev/null
    cp $CONFDIR/install.cfg $CONFDIR/tmp/config/ || err "Error: no install config file"
    cp $CONFDIR/license $CONFDIR/tmp/config/ 2>/dev/null
    cp $CONFDIR/finish $CONFDIR/tmp/config/ 2>/dev/null
    
    cp $CURDIR/internal/startupinstaller.sh $CONFDIR/tmp || err "Error: missing startupinstaller.sh script"
    cp $CURDIR/internal/about $CONFDIR/tmp || err "Error: missing about file"
    
    # Copy languages
    for L in $LANGUAGES
    do
        mkdir -p $CONFDIR/tmp/config/lang/$L/
        cp $CONFDIR/lang/$L/strings $CONFDIR/tmp/config/lang/$L/ 2>/dev/null
        cp $CONFDIR/lang/$L/welcome $CONFDIR/tmp/config/lang/$L/ 2>/dev/null
        cp $CONFDIR/lang/$L/license $CONFDIR/tmp/config/lang/$L/ 2>/dev/null
        cp $CONFDIR/lang/$L/finish $CONFDIR/tmp/config/lang/$L/ 2>/dev/null
    done
 
    # Copy OS specific files
    for OS in $TARGET_OS
    do
        [ -e $CURDIR/bin/$OS ] || err "Error: no binaries found for $OS"
        
        for ARCH in $TARGET_ARCH
        do
            cd $CURDIR/bin/$OS/$ARCH || err "Error: no binaries found for $OS/$ARCH"
            
            for FR in $FRONTENDS
            do
                FRFOUND=0
                FRNAME=
                
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
                    mkdir -p $CONFDIR/tmp/frontends/$OS/$ARCH/$LC
                    cp $LC/$FRNAME $CONFDIR/tmp/frontends/$OS/$ARCH/$LC/ && FROUND=1
                done
    
                cp $FRNAME $CONFDIR/tmp/frontends/$OS/$ARCH/ 2>/dev/null && FRFOUND=1
                if [ $FRFOUND -eq 0 ]; then
                    echo "Warning: no $FR frontend for $OS/$ARCH"
                fi
            done
        done
    done

    # Check if we got an intro picture
    INTROPIC=`awk '$1=="intropic"{print $2}' $CONFDIR/install.cfg`

    if [ ! -z $INTROPIC ]; then
        cp ${CONFDIR}/${INTROPIC} ${CONFDIR}/tmp || echo "Warning: $CONFDIR/$INTROPIC does not exist"
    fi
}

remtemp()
{
    # Clean up all temporarily files
    rm -rf $CONFDIR/tmp
}

# Main part starts here...

# Convert iX86 --> x86
echo $CURRENT_ARCH | grep "i*86" >/dev/null && CURRENT_ARCH="x86"

checkargs

# If target dir has trailing '/', remove it
TEMP=${CONFDIR%*/}
CONFDIR=$TEMP

if [ ! -e $CONFDIR/install.cfg ]; then
    err "Error: no install.cfg found!"
fi

# Check which target frontends there are
FRONTENDS=`awk '$1=="frontends"{for (i=2;i <= NF;i++) printf("%s ", $i) }' $CONFDIR/install.cfg`
if [ -z "$FRONTENDS" ]; then
    FRONTENDS="ncurses fltk" # Change if there are other frontends
fi

# Check which target OS'es there are
TARGET_OS=`awk '$1=="targetos"{for (i=2;i <= NF;i++) printf("%s ", $i) }' $CONFDIR/install.cfg`
if [ -z "$TARGET_OS" ]; then
    TARGET_OS=$CURRENT_OS
fi

# Check which target archs there are
TARGET_ARCH=`awk '$1=="targetarch"{for (i=2;i <= NF;i++) printf("%s ", $i) }' $CONFDIR/install.cfg`
if [ -z "$TARGET_ARCH" ]; then
    TARGET_ARCH=$CURRENT_ARCH
fi

for OS in $TARGET_OS
do
    for ARCH in $TARGET_ARCH
    do
        mkdir -p $CONFDIR/tmp/frontends/$OS/$ARCH
    done
done

mkdir -p $CONFDIR/tmp/config/lang

# Check which archive type to use
ARCH_TYPE=`awk '$1=="archtype"{print $2}' $CONFDIR/install.cfg`
if [ -z "$ARCH_TYPE" ]; then
    ARCH_TYPE="gzip"
fi

# Check which languages to use
LANGUAGES=`awk '$1=="languages"{for (i=2;i <= NF;i++) printf("%s ", $i) }' $CONFDIR/install.cfg`
if [ -z "$LANGUAGES" ]; then
    LANGUAGES="english"
fi

echo
echo "Configuration:"
echo "---------------------------------"
echo "          OS: $TARGET_OS"
echo "       Archs: $TARGET_ARCH"
echo "Archive type: $ARCH_TYPE"
echo "   Languages: $LANGUAGES"
echo "  Config dir: $CONFDIR"
echo "   Frontends: $FRONTENDS"
echo "---------------------------------"
echo
echo

# Copy all files needed by installer(temporary)
copytemp

echo "Generating archive..."
PACKCMD=

case $ARCH_TYPE in
    gzip )
        PACKCMD="tar czf"
        ;;
    bzip2 )
        PACKCMD="tar cjf"
        ;;
    * )
        echo "Error: wrong archive type($ARCH_TYPE). Should be gzip or bzip2"
        exit 1
        ;;
esac

# Pack platform independent files
if [ -d $CONFDIR/files_all ]; then
    cd $CONFDIR/files_all
    $PACKCMD "${CONFDIR}/tmp/${ARCHNAME_BASE}_all" * || err "Couldn't pack files"
fi

# Pack arch dependent files
for ARCH in $TARGET_ARCH
do
    if [ -d ${CONFDIR}/files_all_${ARCH} ]; then
        cd ${CONFDIR}/files_all_${ARCH}
        $PACKCMD ${CONFDIR}/tmp/${ARCHNAME_BASE}_all_${ARCH} * || err "Couldn't pack files"
    fi
done
    
# Pack OS/arch dependent files
for OS in $TARGET_OS
do
    if [ -d ${CONFDIR}/files_${OS}_all ]; then
        cd ${CONFDIR}/files_${OS}_all/
        $PACKCMD ${CONFDIR}/tmp/${ARCHNAME_BASE}_${OS} * || err "Couldn't pack files"
    fi

    for ARCH in $TARGET_ARCH
    do
        if [ -d ${CONFDIR}/files_${OS}_${ARCH} ]; then
            cd ${CONFDIR}/files_${OS}_${ARCH}
            $PACKCMD ${CONFDIR}/tmp/${ARCHNAME_BASE}_${OS}_${ARCH} * || err "Couldn't pack files"
        fi
    done
done

#if [ ! -e "${CONFDIR}/tmp/${ARCHNAME_BASE}_*" ]; then
#    err "Error: no files to install!"
#fi

echo "Generating installer..."
$CURDIR/makeself.sh --$ARCH_TYPE $CONFDIR/tmp $CURDIR/setup.sh "nixstaller" sh ./startupinstaller.sh

echo "Cleaning up..."
#remtemp

echo "Done"

