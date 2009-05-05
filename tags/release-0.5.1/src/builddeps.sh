#!/bin/sh

OS=`uname`
CURRENT_OS=`echo "$OS" | tr [A-Z] [a-z]` # Convert to lowercase
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

if [ $CURRENT_OS != "linux" ]; then
    OPTCFLAGS="-Os"
    OPTLFLAGS=
else
    # Unfortunaly only Linux seems to support these optimzations ... :(
    OPTCFLAGS="-Os -ffunction-sections -fdata-sections"
    OPTLFLAGS="-Wl,--gc-sections"
fi

LFS_CFLAGS=`getconf LFS_CFLAGS 2>/dev/null`

get()
{
    if [ -f "$DESTFILES/`basename $1`" ]; then
        return
    fi
    
    [ -x "`which wget`" ]  && wget -P "$DESTFILES" "$1" && return
    [ -x "`which fetch`" ] && { cd "$DESTFILES" ; fetch "$1" ; cd - ; } && return
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
    get "http://www.gzip.org/zlib/zlib-1.2.3.tar.gz"
    untar "zlib-1.2.3.tar.gz"
    dodir "zlib-1.2.3/"
    CFLAGS="$OPTCFLAGS" ./configure --prefix=$DESTPREFIX && make && make install && make clean
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
    CXXOPTS="$OPTCFLAGS" $MAKE BUILDDIR="$DESTFILES/stdcxx-obj" BUILDMODE=static,optimized 
    dodir stdcxx-obj
    $MAKE install LOCALES="" PREFIX=$DESTPREFIX
    $MAKE clean
    restoredir
}

buildpng()
{
    get "http://prdownloads.sourceforge.net/sourceforge/libpng/libpng-1.2.34.tar.gz"
    untar "libpng-1.2.34.tar.gz"
    dodir "libpng-1.2.34/"
    CFLAGS="$OPTCFLAGS" ./configure --prefix="$DESTPREFIX" --disable-shared && make && make install && make clean
    restoredir
}

buildjpeg()
{
    get "ftp://ftp.uu.net/graphics/jpeg/jpegsrc.v6b.tar.gz"
    untar "jpegsrc.v6b.tar.gz"
    dodir "jpeg-6b"
    CFLAGS="$OPTCFLAGS" ./configure --prefix=$DESTPREFIX && make && make install-lib && make clean
    restoredir
}

buildfltk()
{
#     get "http://ftp.rz.tu-bs.de/pub/mirror/ftp.easysw.com/ftp/pub/fltk/1.1.9/fltk-1.1.9-source.tar.gz"
#     untar "fltk-1.1.9-source.tar.gz"
#     dodir "fltk-1.1.9"
    get "http://ftp.easysw.com/pub/fltk/snapshots/fltk-1.3.x-r6666.tar.gz"
    untar "fltk-1.3.x-r6666.tar.gz"
    dodir "fltk-1.3.x-r6666"

    if [ $CURRENT_OS = "darwin" ]; then
        ./configure --prefix=$DESTPREFIX && make && make install && make clean
    elif [ $CURRENT_OS = "sunos" ]; then
        LIBS="-lXrender -lfreetype -lfontconfig" CFLAGS="$OPTCFLAGS" ./configure --prefix=$DESTPREFIX --without-links --disable-gl --enable-xdbe --enable-xft --disable-xinerama && make && make install && make clean
    else
        CFLAGS="$OPTCFLAGS" ./configure --prefix=$DESTPREFIX --without-links --disable-gl --enable-xdbe --enable-xft --disable-xinerama && make && make install && make clean
    fi
    restoredir
}

buildlua()
{
    get "http://www.lua.org/ftp/lua-5.1.4.tar.gz"
    untar "lua-5.1.4.tar.gz"
    dodir "lua-5.1.4"
    CFLAGS="-Wall -DLUA_USE_POSIX $OPTCFLAGS"
    if [ $CURRENT_OS = "darwin" ]; then
        make macosx CFLAGS="$CFLAGS" && make install INSTALL_TOP=$DESTPREFIX
    else
        make posix CFLAGS="$CFLAGS" && make install INSTALL_TOP=$DESTPREFIX
    fi
    restoredir
}

buildncurses()
{
    get "ftp://invisible-island.net/ncurses/ncurses-5.7.tar.gz"
    untar "ncurses-5.7.tar.gz"
    dodir "ncurses-5.7"
    
    case $CURRENT_OS in
        freebsd | netbsd | openbsd )
            COPTS="--enable-termcap"
            ;;
        sunos )
            COPTS="--with-terminfo-dirs=/usr/share/lib/terminfo/"
            ;;
    esac

    # Examples may fail to build, just make sure to always call make install (no &&).
    CFLAGS="$OPTCFLAGS" ./configure --without-gpm --without-dlsym --enable-widec $COPTS && make ; make install DESTDIR="$DESTPREFIX" && make clean
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

buildxdelta3()
{
    get "http://xdelta.googlecode.com/files/xdelta3.0v2.tar.gz"
    untar "xdelta3.0v2.tar.gz"
    dodir "xdelta3.0v"
    # Don't use Makefile, use some custom flags/settings do decrease bin size a lot
    LFLAGS="$OPTLFLAGS"
    [ $CURRENT_OS = "openbsd" ] && LFLAGS="$LFLAGS -static"
    gcc $OPTCFLAGS xdelta3.c -o xdelta3 -DGENERIC_ENCODE_TABLES=0 -DREGRESSION_TEST=0 -DSECONDARY_DJW=0 -DSECONDARY_FGK=0 -DXD3_DEBUG=0 -DXD3_MAIN=1 -DXD3_POSIX=1 -DXD3_USE_LARGEFILE64=1 $LFLAGS
    strip -s xdelta3
    mkdir -p "$DESTPREFIX/bin"
    cp xdelta3 "$DESTPREFIX/bin"
}

buildlzma()
{
    get "http://heanet.dl.sourceforge.net/sourceforge/sevenzip/lzma457.tar.bz2"
    untar "lzma457.tar.bz2" "bzip2"
    dodir "CPP/7zip/Compress/LZMA_Alone"
    gmake --version >/dev/null 2>&1 && MAKE=gmake || MAKE=make
    LFLAGS="$OPTLFLAGS -static-libgcc"
    [ $CURRENT_OS = "openbsd" ] && LFLAGS="$LFLAGS -static"
	if [ $CURRENT_OS != "linux" -a $CURRENT_OS != "netbsd" -a $CURRENT_OS != "openbsd" ]; then
		$MAKE -f makefile.gcc CXX="g++ $OPTCFLAGS $LFS_CFLAGS" LDFLAGS="$LFLAGS"
	else
		$MAKE -f makefile.gcc CXX="gcc $OPTCFLAGS $LFS_CFLAGS" LDFLAGS="-L$DESTPREFIX/lib $LFLAGS" LIB='-lstd -lsupc++ -lm'
	fi
    mkdir -p "$DESTPREFIX/bin"
    cp lzma "$DESTPREFIX/bin"
    restoredir
    dodir "C/Compress/Lzma"
    patch < "$SRCDIR"/lzmastdout.diff
    gcc $OPTCFLAGS LzmaStateDecode.c LzmaStateTest.c -o lzma-decode $LFLAGS
    cp lzma-decode "$DESTPREFIX/bin"
    restoredir
    [ $CURRENT_OS != "sunos" -a $CURRENT_OS != "darwin" ] && STRIPARGS="-s"
    strip $STRIPARGS "$DESTPREFIX/bin/lzma" "$DESTPREFIX/bin/lzma-decode"
}

buildelf()
{
    get "http://www.mr511.de/software/libelf-0.8.10.tar.gz"
    untar "libelf-0.8.10.tar.gz"
    dodir "libelf-0.8.10"
    CONF=
    if [ $CURRENT_OS = "netbsd" ]; then
        # Use patch from pkgsrc (adds various missing macros)
        patch < "$SRCDIR/elfnbsd.diff"
        # NetBSD's elf.h seems broken...atleast compiling with it results in a not-working
        # libelf
        mv configure configure.orig
        sed 's/libelf_cv_elf_h_works=yes/libelf_cv_elf_h_works=no/g' configure.orig >configure
        chmod +x configure
        CFLAGS="$OPTCFLAGS" ./configure --prefix=$DESTPREFIX --enable-compat
    else
        CFLAGS="$OPTCFLAGS" ./configure --prefix=$DESTPREFIX
    fi
    make && make install && make clean
    restoredir
}

buildcurl()
{
    get "http://curl.haxx.se/download/curl-7.19.3.tar.gz"
    untar "curl-7.19.3.tar.gz"
    dodir "curl-7.19.3"
    ./configure CFLAGS="$OPTCFLAGS" --prefix=$DESTPREFIX --enable-static --disable-shared --disable-file --disable-ldap --disable-ldaps --disable-dict --disable-telnet --disable-thread --disable-ares --disable-debug --disable-crypto-auth --disable-cookies --without-ssl --without-libssh2 --without-libidn && make && make install && make clean
    restoredir
}

buildxft()
{
    get "http://xorg.freedesktop.org/releases/individual/lib/libXft-2.1.11.tar.gz"
    untar "libXft-2.1.11.tar.gz"
    dodir "libXft-2.1.11"
    CFLAGS="$OPTCFLAGS" ./configure --prefix=$DESTPREFIX --x-includes=/usr/include/X11/ && make && make install && make clean
    restoredir
}

BUILD="$*"

if [ -z "$BUILD" ]; then
	if [ $CURRENT_OS = "darwin" ]; then
		BUILD="png jpeg fltk lua ncurses lzma elf curl"
    elif [ $CURRENT_OS = "sunos" ]; then
        BUILD="zlib png jpeg xft fltk lua ncurses lzma elf curl"
    else
        BUILD="zlib png jpeg fltk lua ncurses lzma elf curl"
        [ $CURRENT_OS != "freebsd" ] && BUILD="stdcxx $BUILD"
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
        xdelta3 ) buildxdelta3 ;;
        lzma ) buildlzma ;;
        elf ) buildelf ;;
        curl ) buildcurl ;;
        xft ) buildxft ;;
        * ) echo "Wrong build option" ; exit 1 ;;
    esac
done

