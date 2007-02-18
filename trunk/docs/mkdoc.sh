#!/bin/sh
#makeinfo --html --css-include=styles.css nixstaller.texi
texi2html nixstaller.texi -split=chapter -subdir=html --css-include=styles.css
