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

FILESDIR=
FILEPREFIX="plist_"
REL="extrpath"

usage()
{
    echo "Usage: $0 <file dir>"
    echo
    echo " <file dir>: The directory which holds the files that are going to be on the users system"
    exit 1
}

if [ -z "$1" ]; then
    usage
fi

while true
do
    case $1 in
        --rel | -r)
            REL=$2
            shift 2
            ;;
        --help | -h)
            usage
            ;;
        * )
            break
            ;;
    esac
done

if [ -z "$1" ]; then
    usage
fi

FILESDIR="$1"

if [ ! -d "$FILESDIR" ]; then
    echo "No such directory: ${FILESDIR} 22"
    exit 1
fi

# If target dir has trailing '/', remove it
FILESDIR=${FILESDIR%*/}

LISTNAME="${FILEPREFIX}${REL}"

rm -f ${LISTNAME}
ls -A "${FILESDIR}" >> "${LISTNAME}"

echo "Generated list file ($PWD/$LISTNAME). Please put it in your installer configuration directory if it's not already there."
