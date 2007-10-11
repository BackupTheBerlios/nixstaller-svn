#!/bin/sh

# Use some SH first to bootstrap...

FRONTENDS="gtk fltk ncurs"
CURDIR=`pwd`

OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]` # Convert to lowercase

CURRENT_ARCH=`uname -m`
echo $CURRENT_ARCH | grep "i.86" >/dev/null && CURRENT_ARCH="x86" # Convert iX86 --> x86
echo $CURRENT_ARCH | grep "86pc" >/dev/null && CURRENT_ARCH="x86" # Convert 86pc --> x86

haslibs()
{
    if [ $CURRENT_OS = "openbsd" ]; then
        (ldd $1 >/dev/null 2>&1) && return 0
    else
        if [ -z "`ldd $1 | grep 'not found'`" ]; then
           return 0
        fi
    fi
    return 1
}

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

for LC in `ls -d bin/${CURRENT_OS}/${CURRENT_ARCH}/libc* 2>/dev/null | sort -nr`
do
    for LCPP in `ls -d ${LC}/'libstdc++'* 2>/dev/null | sort -nr`
    do
        for F in $FRONTENDS
        do
            BIN=${LCPP}/$F
            if [ -f $BIN ]; then
                haslibs $BIN || continue
                $BIN -c "$CURDIR/src/internal/geninstall.lua" "$@" || exit 1
                exit 0
            fi
        done
    done
done

echo "Could not find a suitable binary for this platform ($CURRENT_ARCH, $CURRENT_OS)"
exit 1
