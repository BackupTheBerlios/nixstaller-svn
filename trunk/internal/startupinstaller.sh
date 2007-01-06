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


# Does some simple checking, and tries to launch the right installer frontend

# Unpacks $1.lzma to lzma and removes $1.lzma
# $1: File to handle
# $2: libc directory to be used for lzma
unlzma()
{
    if [ ! -z "$1" -a -e $1.lzma ]; then
        $2/lzma-decode $1.lzma $1 2>&1 >/dev/null && rm $1.lzma
    fi
}

configure()
{
    echo "Collecting info for this system..."
    
    OS=`uname`
    CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`
    echo "Operating system: $CURRENT_OS"

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
  
    # Get all C libs. Sorted so higher versions come first
    LIBCS=`echo /lib/libc.so.* | sort -nr`
    echo "C libraries: $LIBCS"
    
    # Get all C++ libs. Sorted so higher versions come first
    LIBSTDCPPS=`echo '/usr/lib/libstdc++.so.'* | sort -nr`
    echo "C++ libraries: $LIBSTDCPPS"
}

# Uses edelta to reconstruct frontend binaries
# $1: libc directory to be used(for lzma and edelta)
# $2: source file
# $3: diff file
edelta()
{
    unlzma $3
    
    mv $3 $3.tmp
    $1/edelta -q patch $2 $3 $3.tmp >/dev/null
}

# Check which archive type to use
# ARCH_TYPE=`awk '$1=="archtype"{print $2}' config/install.cfg`
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
        LCDIR="bin/$CURRENT_OS/$CURRENT_ARCH/"`basename $LC`
        echo "Trying libc for $FR: " $LCDIR
        
        if [ ! -d ${LCDIR} ]; then
            continue
        fi
    
        if [ $ARCH_TYPE = "lzma" -a ! -e ${LCDIR}/lzma-decode ]; then
            continue # No usable lzma-decoder
        fi
        
        unlzma $ED_SRC ${LCDIR}
        
        for LCPP in $LIBSTDCPPS
        do
            LCPPDIR=${LCDIR}/`basename $LCPP`
            echo "Trying libstdc++ for $FR: " $LCPPDIR
            
            if [ ! -d ${LCPPDIR} ]; then
                continue
            fi
            
            unlzma ${LCPPDIR}/$FR ${LCDIR}
            
            if [ -e ${LCPPDIR}/$FR ]; then
                FRONTEND="${LCPPDIR}/$FR"
                if [ ! -z "$ED_SRC" -a $FRONTEND != $ED_SRC ]; then
                    edelta ${LCDIR} $ED_SRC $FRONTEND
                fi
                
                # Run it
                chmod +x $FRONTEND # deltas en lzma packed bins probably aren't executable yet
                ./$FRONTEND
                exit $?
            fi
        done
    done
done

echo "Error: Couldn't find any suitable frontend for your system"
exit 1
