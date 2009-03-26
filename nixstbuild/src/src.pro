SOURCES += nixstbuild.cpp \
           main.cpp \
           sdialog.cpp \
           rungen.cpp \
           luahl.cpp \
           bdialog.cpp \
 cvaroutput.cpp \
 nlist.cpp \
 n2list.cpp \
 nbstandard.cpp \
 modechooser.cpp
HEADERS += nixstbuild.h \
sdialog.h \
rungen.h \
luahl.h \
bdialog.h \
 cvaroutput.h \
 nlist.h \
 n2list.h \
 nbstandard.h \
 modechooser.h
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 debug
TARGET = ../bin/nixstbuild
RESOURCES = application.qrc
FORMS += textfedit.ui \
screeninput.ui
CONFIG -= release

