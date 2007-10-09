#!/bin/sh

DESTFILES=$PWD/deps/files
DESTPREFIX=$PWD/deps/usr
mkdir -p "$DESTFILES"
mkdir -p "$DESTPREFIX"

export LDFLAGS="-L $DESTPREFIX/lib"
export CPPFLAGS="-I $DESTPREFIX/include"

get()
{
    if [ -f "$DESTFILES/`basename $1`" ]; then
        return
    fi
    
    wget -P "$DESTFILES" "$1" && return
#    fetch $2 && return
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

buildgzip()
{
    get "ftp://ftp.nluug.nl/pub/gnu/gzip/gzip-1.2.4.tar.gz" "$DESTFILES"
    untar "gzip-1.2.4.tar.gz"
    dodir "gzip-1.2.4/"
    ./configure --prefix=$DESTPREFIX && make && make install && make clean
    cd -
}

buildpng()
{
    get "http://belnet.dl.sourceforge.net/sourceforge/libpng/libpng-1.2.21.tar.gz"
    untar "libpng-1.2.21.tar.gz"
    dodir "libpng-1.2.21/"
    ./configure --prefix="$DESTPREFIX" && make && make install && make clean
    cd -
}

buildjpeg()
{
    get "http://freshmeat.net/redir/libjpeg/5665/url_tgz/jpegsrc.v6b.tar.gz"
    untar "jpegsrc.v6b.tar.gz"
    dodir "jpeg-6b"
    ./configure --prefix=$DESTPREFIX && make && make install && make clean
    cd -
}

buidfltk()
{
    get "http://ftp.rz.tu-bs.de/pub/mirror/ftp.easysw.com/ftp/pub/fltk/1.1.7/fltk-1.1.7-source.tar.gz"
    untar "fltk-1.1.7-source.tar.gz"
    dodir "fltk-1.1.7"
    CXXFLAGS="-fexceptions" ./configure --prefix=$DESTPREFIX --enable-xft --enable-xdbe && make && make install && make clean
    cd -
}

buildgzip
buildpng
buildjpeg
buidfltk
