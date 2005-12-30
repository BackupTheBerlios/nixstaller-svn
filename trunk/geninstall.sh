#!/bin/sh

ARGS="$*"
CURDIR=$PWD
ARCHNAME_BASE="instarchive"
OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`
CONFDIR="$CURDIR/$1"
TARGET_OS=

err()
{
    echo $1
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
    
    cp $CURDIR/startupinstaller.sh $CONFDIR/tmp || err "Error: missing startupinstaller.sh script"
    
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
        cp $CURDIR/bin/$OS/ncurs $CONFDIR/tmp/frontends/$OS/ || echo "Warning: no ncurses frontend for $OS"
        cp $CURDIR/bin/$OS/fltk $CONFDIR/tmp/frontends/$OS/ || echo "Warning: no fltk frontend for $OS"
        cp $CURDIR/lib/$OS/libc.so.* $CONFDIR/tmp/lib/$OS/ || echo "Warning: missing libc for $OS"
        cp $CURDIR/lib/$OS/libm.so.* $CONFDIR/tmp/lib/$OS/ || echo "Warning: missing libm for $OS"
    done
}

remtemp()
{
    # Clean up all temporarily files
    rm -rf $CONFDIR/tmp
}

# Main part starts here...

checkargs

# Check which target OS'es there are
TARGET_OS=`awk '$1=="targetos"{for (i=2;i <= NF;i++) printf("%s ", $i) }' $CONFDIR/install.cfg`
if [ -z "$TARGET_OS" ]; then
    TARGET_OS=$CURRENT_OS
fi

for OS in $TARGET_OS
do
    mkdir -p $CONFDIR/tmp/lib/$OS
    mkdir -p $CONFDIR/tmp/frontends/$OS
done

mkdir -p $CONFDIR/tmp/config/lang

# Check which archive type to use
ARCH_TYPE=`awk '$1=="archtype"{print $2}' $CONFDIR/install.cfg`

# Check which languages to use
LANGUAGES=`awk '$1=="languages"{for (i=2;i <= NF;i++) printf("%s ", $i) }' $CONFDIR/install.cfg`
if [ -z "$LANGUAGES" ]; then
    LANGUAGES="english"
fi

echo
echo "Configuration:"
echo "---------------------------------"
echo "          OS: $TARGET_OS"
echo "Archive type: $ARCH_TYPE"
echo "   Languages: $LANGUAGES"
echo "  Config dir: $CONFDIR"
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
        PACKCMD="tar czf"
        ;;
    * )
        echo "Error: wrong archive type($ARCH_TYPE). Should be gzip or bzip2"
        exit 1
        ;;
esac

if [ -d $CONFDIR/files_all ]; then
    cd $CONFDIR/files_all
    $PACKCMD "$CONFDIR"/tmp/"$ARCHNAME_BASE"_all * || err "Couldn't pack files"
fi

for OS in $TARGET_OS
do
    if [ -d "$CONFDIR"/files_"$OS" ]; then
        cd $CONFDIR/files_$OS 
        $PACKCMD "$CONFDIR"/tmp/"$ARCHNAME_BASE"_"$OS" * || err "Couldn't pack files"
    fi
done

if [ ! -d $CONFDIR/files_all -a ! -d $CONFDIR/files_$CURRENT_OS ]; then
    err "Error: no files to install!"
fi

echo "Generating installer..."
$CURDIR/makeself.sh $CONFDIR/tmp $CURDIR/setup.sh "nixstaller" sh ./startupinstaller.sh

echo "Cleaning up..."
remtemp

echo "Done"

