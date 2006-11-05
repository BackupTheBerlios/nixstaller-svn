#/bin/sh

# Use some SH first to bootstrap...

FRONTENDS="fltk ncurs"
CURDIR=$PWD

OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]` # Convert to lowercase

CURRENT_ARCH=`uname -m`
echo $CURRENT_ARCH | grep "i*86" >/dev/null && CURRENT_ARCH="x86" # Convert iX86 --> x86
echo $CURRENT_ARCH | grep "86pc" >/dev/null && CURRENT_ARCH="x86" # Convert 86pc --> x86

for LC in `ls -d bin/${CURRENT_OS}/${CURRENT_ARCH}/libc* 2>/dev/null | sort -nr`
do
    for LCPP in `ls -d ${LC}/'libstdc++'* 2>/dev/null | sort -nr`
    do
        for BIN in ${LCPP}/$FRONTENDS
        do
            if [ -e $BIN ]; then
                if [ ! `ldd  $BIN | grep "not found"` ]; then
                    $BIN -c "$CURDIR/internal/geninstall.lua" $* || exit 1
                    exit 0
                fi
            fi
        done
    done
done
