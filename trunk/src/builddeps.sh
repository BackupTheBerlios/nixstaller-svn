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
    [ "$2" = "bzip2" ] && UNCOMP=bzip2 || UNCOMP=gzip
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

buildstdcxx()
{
    get "http://www.apache.org/dist/stdcxx/stdcxx-4.2.1.tar.gz"
    untar "stdcxx-4.2.1.tar.gz"
#     dodir "stdcxx-4.2.0/include/ansi"
#     patch < "$SRCDIR"/stdcxx-gcc43.diff
    dodir "stdcxx-4.2.1"
    gmake --version >/dev/null 2>&1 && MAKE=gmake || MAKE=make
    [ $CURRENT_OS = "netbsd" ] && ulimit -d 131072
    $MAKE BUILDDIR="$DESTFILES/stdcxx-obj" BUILDMODE=static,optimized 
    dodir stdcxx-obj
    $MAKE install LOCALES="" PREFIX=$DESTPREFIX
    $MAKE clean
    restoredir
}

buildpng()
{
    get "http://prdownloads.sourceforge.net/sourceforge/libpng/libpng-1.2.21.tar.gz"
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
    get "http://ftp.rz.tu-bs.de/pub/mirror/ftp.easysw.com/ftp/pub/fltk/1.1.9/fltk-1.1.9-source.tar.gz"
    untar "fltk-1.1.9-source.tar.gz"
    dodir "fltk-1.1.9"
    if [ $CURRENT_OS = "darwin" ]; then
        CXXFLAGS="-fexceptions" ./configure --prefix=$DESTPREFIX && make && make install && make clean
    else
        CXXFLAGS="-fexceptions" ./configure --prefix=$DESTPREFIX --enable-xdbe && make && make install && make clean
    fi
    restoredir
}

buildlua()
{
    get "http://www.lua.org/ftp/lua-5.1.4.tar.gz"
    untar "lua-5.1.4.tar.gz"
    dodir "lua-5.1.4"
    CFLAGS="-Os -Wall -DLUA_USE_POSIX"
    if [ $CURRENT_OS = "darwin" ]; then
        make macosx CFLAGS="$CFLAGS" && make install INSTALL_TOP=$DESTPREFIX
    else
        make posix CFLAGS="$CFLAGS" && make install INSTALL_TOP=$DESTPREFIX
    fi
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
    $MAKE -f makefile.gcc CXX='gcc -Os' LDFLAGS="-L$DESTPREFIX/lib" LIB='-lstd -lsupc++ -lm'
    mkdir -p "$DESTPREFIX/bin"
    cp lzma "$DESTPREFIX/bin"
    restoredir
    dodir "C/Compress/Lzma"
    patch < "$SRCDIR"/lzmastdout.diff
    gcc -Os LzmaStateDecode.c LzmaStateTest.c -o lzma-decode
    cp lzma-decode "$DESTPREFIX/bin"
    restoredir
    [ $CURRENT_OS != "sunos" ] && STRIPARGS="-s"
    strip $STRIPARGS "$DESTPREFIX/bin/lzma" "$DESTPREFIX/bin/lzma-decode"
}

buildelf()
{
    get "http://www.mr511.de/software/libelf-0.8.10.tar.gz"
    untar "libelf-0.8.10.tar.gz"
    dodir "libelf-0.8.10"
    ./configure --prefix=$DESTPREFIX && make && make install && make clean
    restoredir
}

buildcurl()
{
    get "http://curl.haxx.se/download/curl-7.18.0.tar.gz"
    untar "curl-7.18.0.tar.gz"
    dodir "curl-7.18.0"
    ./configure --prefix=$DESTPREFIX --enable-static --disable-file --disable-ldap --disable-ldaps --disable-dict --disable-telnet --disable-thread --disable-ares --disable-debug --disable-crypto-auth --disable-cookies --without-ssl --without-libssh2 --without-libidn && make && make install && make clean
    restoredir
}

BUILD="$*"

if [ -z "$BUILD" ]; then
	if [ $CURRENT_OS = "darwin" ]; then
		BUILD="png jpeg fltk lua ncurses lzma elf curl"
	else
	    BUILD="zlib stdcxx png jpeg fltk lua ncurses lzma elf curl"
	fi
#     if [ `uname` = "Linux" ]; then
#         BUILD="$BUILD beecrypt rpm"
#     fi
fi

for B in $BUILD
do
    case $B in
        stdcxx ) buildstdcxx ;;
        zlib ) buildzlib ;;
        png ) buildpng ;;
        jpeg ) buildjpeg ;;
        fltk ) buildfltk ;;
        lua ) buildlua ;;
        ncurses ) buildncurses ;;
        beecrypt ) buildbeecrypt ;;
        rpm ) buildrpm ;;
        lzma ) buildlzma ;;
        elf ) buildelf ;;
        curl ) buildcurl ;;
        * ) echo "Wrong build option" ; exit 1 ;;
    esac
done

