#!/bin/sh

SRCDIR=$PWD
DESTFILES=$PWD/deps/files
DESTPREFIX=$PWD/deps/usr
mkdir -p "$DESTFILES"
mkdir -p "$DESTPREFIX"

LDFLAGS="-L $DESTPREFIX/lib"
CPPFLAGS="-I $DESTPREFIX/include"
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
    (cd "$DESTFILES/" && cat "$1" | gzip -cd | tar xvf -)
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
    get "http://freshmeat.net/redir/zlib/12352/url_tgz/zlib-1.2.3.tar.gz" "$DESTFILES"
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
    ./configure --prefix="$DESTPREFIX" && make && make install && make clean
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
    CXXFLAGS="-fexceptions" ./configure --prefix=$DESTPREFIX --enable-xft --enable-xdbe && make && make install && make clean
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
    ./configure --without-gpm --without-dlsym && make #&&  && make clean
    restoredir
}

BUILD=$1

if [ -z $BUILD ]; then
    BUILD="zlib png jpeg fltk lua ncurses"
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
    esac
done

