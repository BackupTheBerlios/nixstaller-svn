#!/bin/sh
#makeinfo --html --css-include=styles.css nixstaller.texi
texi2html nixstaller.texi -split=chapter -subdir=nixstaller-texi --css-include=styles.css
