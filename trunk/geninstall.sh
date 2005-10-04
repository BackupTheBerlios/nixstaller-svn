#!/bin/sh

# yes this script could be better...
# I like C++ more ;-)

CURDIR=$PWD
ARCH_GZIP="instarchive.tar.gz"
ARCH_BZIP2="instarchive.tar.bz2"
OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`

copytemp()
{
    # Temporarily copy frontends, libs and install config files to install directory
    cp $CURDIR/bin/$CURRENT_OS/ncurs $CURDIR/release/tmp/frontends/$CURRENT_OS/
    cp $CURDIR/release/welcome $CURDIR/release/tmp/config/
    cp $CURDIR/release/install.cfg $CURDIR/release/tmp/config/
    if [ -e $CURDIR/release/license ]; then
        cp $CURDIR/release/license $CURDIR/release/tmp/config/
    fi
    
    # Copy all required libs
    cp $CURDIR/lib/$CURRENT_OS/libc.so.* $CURDIR/release/tmp/lib/$CURRENT_OS/
    cp $CURDIR/lib/$CURRENT_OS/libm.so.* $CURDIR/release/tmp/lib/$CURRENT_OS/
    cp $CURDIR/lib/$CURRENT_OS/libstdc++.so.* $CURDIR/release/tmp/lib/$CURRENT_OS/
    
    cp $CURDIR/startupinstaller.sh $CURDIR/release/tmp
}

remtemp()
{
    # Clean up all temporarily files
    rm -rf $CURDIR/release/tmp
}

# Main part starts here...

echo "Making installer for following OS: $CURRENT_OS"

mkdir -p $CURDIR/release/tmp/lib/$CURRENT_OS
mkdir -p $CURDIR/release/tmp/frontends/$CURRENT_OS
mkdir -p $CURDIR/release/tmp/config

# Check which archive type to use
ARCH_TYPE=`awk '$1=="archtype"{print $2}' release/install.cfg`
echo "Using archive type $ARCH_TYPE..."

echo "Generating archive..."
cd $CURDIR/release/files

case $ARCH_TYPE in
    gzip )
        tar czf $CURDIR/release/tmp/$ARCH_GZIP *
        ;;
    bzip2 )
        tar cjf $CURDIR/release/tmp/$ARCH_BZIP2 *
        ;;
    * )
        echo "Error: wrong archive type($ARCH_TYPE)"
        exit 1
        ;;
esac

# Copy all files needed by installer(temporalily)
copytemp

echo "Generating installer..."
$CURDIR/makeself.sh $CURDIR/release/tmp $CURDIR/setup.sh "nixstaller" sh ./startupinstaller.sh

echo "Cleaning up..."
remtemp

echo "Done"

