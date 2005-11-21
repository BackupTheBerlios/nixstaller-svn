#!/bin/sh

ARGS="$*"
CURDIR=$PWD
ARCH_NAME="instarchive"
OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`
CONFDIR="$CURDIR/$1"

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
    # Temporarily copy frontends, libs and install config files to install directory
    cp $CURDIR/bin/$CURRENT_OS/ncurs $CONFDIR/tmp/frontends/$CURRENT_OS/
    cp $CURDIR/bin/$CURRENT_OS/fltk $CONFDIR/tmp/frontends/$CURRENT_OS/
    cp $CONFDIR/welcome $CONFDIR/tmp/config/
    cp $CONFDIR/install.cfg $CONFDIR/tmp/config/
    if [ -e $CONFDIR/license ]; then
        cp $CONFDIR/license $CONFDIR/tmp/config/
    fi
    
    # Copy languages
    for L in $LANGUAGES
    do
        mkdir -p $CONFDIR/tmp/config/lang/$L/
        cp $CURDIR/lang/$L/strings $CONFDIR/tmp/config/lang/$L/
        cp $CONFDIR/lang/$L/welcome $CONFDIR/tmp/config/lang/$L/
        cp $CONFDIR/lang/$L/license $CONFDIR/tmp/config/lang/$L/
    done
    
    # Copy all required libs
    cp $CURDIR/lib/$CURRENT_OS/libc.so.* $CONFDIR/tmp/lib/$CURRENT_OS/
    cp $CURDIR/lib/$CURRENT_OS/libm.so.* $CONFDIR/tmp/lib/$CURRENT_OS/
    
    cp $CURDIR/startupinstaller.sh $CONFDIR/tmp
}

remtemp()
{
    # Clean up all temporarily files
    rm -rf $CONFDIR/tmp
}

# Main part starts here...

checkargs

mkdir -p $CONFDIR/tmp/lib/$CURRENT_OS
mkdir -p $CONFDIR/tmp/frontends/$CURRENT_OS
mkdir -p $CONFDIR/tmp/config/lang

# Check which archive type to use
ARCH_TYPE=`awk '$1=="archtype"{print $2}' $CONFDIR/install.cfg`

LANGUAGES=`awk '$1=="languages"{for (i=2;i <= NF;i++) printf("%s ", $i) }' $CONFDIR/install.cfg`
if [ -z "$LANGUAGES" ]; then
    LANGUAGES="english"
fi

echo
echo "Configuration:"
echo "---------------------------------"
echo "          OS: $CURRENT_OS"
echo "Archive type: $ARCH_TYPE"
echo "   Languages: english $LANGUAGES"
echo "  Config dir: $CONFDIR"
echo "---------------------------------"
echo
echo

echo "Generating archive..."
cd $CONFDIR/files

case $ARCH_TYPE in
    gzip )
        tar czf $CONFDIR/tmp/$ARCH_NAME *
        ;;
    bzip2 )
        tar cjf $CONFDIR/tmp/$ARCH_NAME *
        ;;
    * )
        echo "Error: wrong archive type($ARCH_TYPE)"
        exit 1
        ;;
esac

# Copy all files needed by installer(temporalily)
copytemp

echo "Generating installer..."
$CURDIR/makeself.sh $CONFDIR/tmp $CURDIR/setup.sh "nixstaller" sh ./startupinstaller.sh

echo "Cleaning up..."
remtemp

echo "Done"

