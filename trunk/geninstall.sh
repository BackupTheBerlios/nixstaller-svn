#/bin/sh

# Use some SH first to bootstrap...

FRONTENDS="fltk ncurs"
CURDIR=$PWD

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

for LC in `ls -d bin/${CURRENT_OS}/${CURRENT_ARCH}/libc* 2>/dev/null | sort -nr`
do
    for LCPP in `ls -d ${LC}/'libstdc++'* 2>/dev/null | sort -nr`
    do
        for F in $FRONTENDS
        do
            BIN=${LCPP}/$F
            if [ -f $BIN ]; then
                haslibs $BIN || continue
                $BIN -c "$CURDIR/internal/geninstall.lua" $* || exit 1
                exit 0
            fi
        done
    done
done

echo "Could not find a suitable binary for this platform ($CURRENT_ARCH, $CURRENT_OS)"
exit 1
