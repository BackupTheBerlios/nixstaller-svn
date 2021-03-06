#!/bin/sh

OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`
CURRENT_ARCH=`uname -m`
echo $CURRENT_ARCH | grep "i.86" >/dev/null && CURRENT_ARCH="x86"
echo $CURRENT_ARCH | grep "i86pc" >/dev/null && CURRENT_ARCH="x86"   
echo $CURRENT_ARCH | grep "amd64" >/dev/null && CURRENT_ARCH="x86_64"
	
SRCDIR=$PWD
DESTFILES=$PWD/deps/files
DESTPREFIX=$PWD/deps/usr
mkdir -p "$DESTFILES"
mkdir -p "$DESTPREFIX"

LDFLAGS="-L$DESTPREFIX/lib -L$DESTPREFIX/lib64"
CPPFLAGS="-I$DESTPREFIX/include"
export LDFLAGS
export CPPFLAGS

get()
{
    if [ -f "$DESTFILES/`basename $1`" ]; then
        return
    fi
    
    wget -P "$DESTFILES" "$1" && return
    cd "$DESTFILES" ; fetch "$1" ; cd - ; return
    echo "Download file manually"
}

untar()
{
    [ $2 = "bzip2" ] && UNCOMP=bzip2 || UNCOMP=gzip
    (cd "$DESTFILES/" && cat "$1" | $UNCOMP -cd | tar xvf -)
}

dodir()
{
    cd "$DESTFILES/$1"
}

restoredir()
{
    cd $SRCDIR
}

buildzlib()
{
    get "http://freshmeat.net/redir/zlib/12352/url_tgz/zlib-1.2.3.tar.gz"
    untar "zlib-1.2.3.tar.gz"
    dodir "zlib-1.2.3/"
    ./configure --prefix=$DESTPREFIX && make && make install && make clean
    restoredir
}

buildpng()
{
    get "http://belnet.dl.sourceforge.net/sourceforge/libpng/libpng-1.2.21.tar.gz"
    untar "libpng-1.2.21.tar.gz"
    dodir "libpng-1.2.21/"
    ./configure --prefix="$DESTPREFIX" --disable-shared && make && make install && make clean
    restoredir
}

buildjpeg()
{
    get "http://freshmeat.net/redir/libjpeg/5665/url_tgz/jpegsrc.v6b.tar.gz"
    untar "jpegsrc.v6b.tar.gz"
    dodir "jpeg-6b"
    ./configure --prefix=$DESTPREFIX && make && make install-lib && make clean
    restoredir
}

buildfltk()
{
    get "http://ftp.rz.tu-bs.de/pub/mirror/ftp.easysw.com/ftp/pub/fltk/1.1.7/fltk-1.1.7-source.tar.gz"
    untar "fltk-1.1.7-source.tar.gz"
    dodir "fltk-1.1.7"
    CXXFLAGS="-fexceptions" ./configure --prefix=$DESTPREFIX --enable-xdbe && make && make install && make clean
    restoredir
}

buildlua()
{
    get "http://www.lua.org/ftp/lua-5.1.2.tar.gz"
    untar "lua-5.1.2.tar.gz"
    dodir "lua-5.1.2"
    nano -w src/Makefile
    make posix && make install INSTALL_TOP=$DESTPREFIX
    restoredir
}

buildncurses()
{
    get "ftp://invisible-island.net/ncurses/ncurses-5.6.tar.gz"
    untar "ncurses-5.6.tar.gz"
    dodir "ncurses-5.6"
    
    case $CURRENT_OS in
        freebsd | netbsd | openbsd )
            COPTS="--enable-termcap"
            ;;
        sunos )
            COPTS="--with-terminfo-dirs=/usr/share/lib/terminfo/"
            ;;
    esac

    # Examples may fail to build, just make sure to always call make install (no &&).
    ./configure --without-gpm --without-dlsym $COPTS && make ; make install DESTDIR="$DESTPREFIX" && make clean
    restoredir
}

buildbeecrypt()
{
    get "http://belnet.dl.sourceforge.net/sourceforge/beecrypt/beecrypt-4.1.2.tar.gz"
    untar "beecrypt-4.1.2.tar.gz"
    dodir "beecrypt-4.1.2"
    CFLAGS="-fPIC" ./configure --prefix=$DESTPREFIX --without-python --disable-shared && make && make install && make clean
    restoredir
}

buildrpm()
{
    get "http://www.rpm.org/releases/rpm-4.4.x/rpm-4.4.2.2.tar.gz"
    untar "rpm-4.4.2.2.tar.gz"
    dodir "rpm-4.4.2.2"
    # Move libz.a temporary away (gives compile error when trying to link static version)
    [ -f "$DESTPREFIX/lib/libz.a" ] && mv "$DESTPREFIX/lib/libz.a" "$DESTPREFIX/lib/libz.a-"
    CPPFLAGS="$CPPFLAGS -I$DESTPREFIX/include/beecrypt" ./configure --without-selinux --without-python --disable-shared --prefix=$DESTPREFIX
    echo '#define HAVE_BEECRYPT_API_H 1' >> config.h
    make  && make install && make clean
    [ -f "$DESTPREFIX/lib/libz.a-" ] && mv "$DESTPREFIX/lib/libz.a-" "$DESTPREFIX/lib/libz.a"
    restoredir
}

buildlzma()
{
    get "http://heanet.dl.sourceforge.net/sourceforge/sevenzip/lzma457.tar.bz2"
    untar "lzma457.tar.bz2" "bzip2"
    dodir "CPP/7zip/Compress/LZMA_Alone"
    gmake --version >/dev/null 2>&1 && MAKE=gmake || MAKE=make
    $MAKE -f makefile.gcc
    mkdir -p "$DESTPREFIX/bin"
    cp lzma "$DESTPREFIX/bin"
    restoredir
    dodir "C/Compress/Lzma"
    gcc -Os LzmaStateDecode.c LzmaStateTest.c -o lzma-decode
    cp lzma-decode "$DESTPREFIX/bin"
    restoredir
    [ $CURRENT_OS != "sunos" ] && STRIPARGS="-s"
    strip $STRIPARGS "$DESTPREFIX/bin/lzma" "$DESTPREFIX/bin/lzma-decode"
}

BUILD="$*"

if [ -z $BUILD ]; then
    BUILD="zlib png jpeg fltk lua ncurses lzma"
#     if [ `uname` = "Linux" ]; then
#         BUILD="$BUILD beecrypt rpm"
#     fi
fi

for B in $BUILD
do
    case $B in
        zlib ) buildzlib ;;
        png ) buildpng ;;
        jpeg ) buildjpeg ;;
        fltk ) buildfltk ;;
        lua ) buildlua ;;
        ncurses ) buildncurses ;;
        beecrypt ) buildbeecrypt ;;
        rpm ) buildrpm ;;
        lzma ) buildlzma ;;
        * ) echo "Wrong build option" ; exit 1 ;;
    esac
done

