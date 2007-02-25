#!/bin/sh
#makeinfo --html --css-include=styles.css nixstaller.texi
OUTDIR=../../doc
texi2html nixstaller.texi --split=chapter --subdir=$OUTDIR --css-include=styles.css --no-sec-nav
cp fltk.png ncurs.png $OUTDIR
