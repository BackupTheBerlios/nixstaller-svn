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

configure()
{
    echo "Collecting info for this system..."
    
    OS=`uname`
    CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`
    echo "Operating system: $CURRENT_OS"
    
    CURRENT_ARCH=`uname -m`
    # iX86 --> x86
    echo $CURRENT_ARCH | grep "i*86" >/dev/null && CURRENT_ARCH="x86"
    echo "CPU Arch: $CURRENT_ARCH"
    
    # Get all C libs. Sorted so higher versions come first
    LIBCS=`ls /lib/libc.so.* | sort -nr`
    echo "C libraries: "`echo $LIBCS | tr 'n\' ' '`
    
    # Get all C++ libs. Sorted so higher versions come first
    LIBSTDCPPS=`ls '/usr/lib/libstdc++.so.'* | sort -nr`
    echo "C++ libraries: "`echo $LIBSTDCPPS | tr 'n\' ' '`
}

# Check which archive type to use
ARCH_TYPE=`awk '$1=="archtype"{print $2}' config/install.cfg`
if [ -z "$ARCH_TYPE" ]; then
    ARCH_TYPE="gzip"
fi

configure

# Try to launch a frontend
for LC in $LIBCS
do
    LCDIR="./bin/$CURRENT_OS/$CURRENT_ARCH/"`basename $LC`
    echo "Trying libc: " $LCDIR
    
    if [ ! -d ${LCDIR} ]; then
        continue
    fi

    for LCPP in $LIBSTDCPPS
    do
        LCPPDIR=${LCDIR}/`basename $LCPP`
        echo "Trying libstdc++: " $LCPPDIR
        
        if [ ! -d ${LCPPDIR} ]; then
            continue
        fi
        
        # X Running?
        if [ ! -z $DISPLAY ]; then
            FRONTEND="${LCPPDIR}/fltk"
        else
            FRONTEND="${LCPPDIR}}/ncurs"
        fi
        
        # If the package is compressed with lzma, the frontends will be aswell. So unpack them if required
        if [ $ARCH_TYPE = "lzma" ]; then
            ${LCDIR}/lzma-decode $FRONTEND ${FRONTEND}.2
            FRONTEND=${FRONTEND}.2
            chmod +x $FRONTEND
        fi
        
        # Run it
        $FRONTEND inst
        exit 0
    done
done

echo "Error: Couldn't find any suitable frontend for your system"
exit 1
