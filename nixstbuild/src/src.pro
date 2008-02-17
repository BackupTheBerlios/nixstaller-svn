SOURCES += nixstbuild.cpp \
           main.cpp \
           sdialog.cpp \
           rungen.cpp \
           luahl.cpp \
           bdialog.cpp \
 cvaroutput.cpp \
 nlist.cpp \
 n2list.cpp
HEADERS += nixstbuild.h \
sdialog.h \
rungen.h \
luahl.h \
bdialog.h \
 cvaroutput.h \
 nlist.h \
 n2list.h
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
TARGET = ../bin/nixstbuild
RESOURCES = application.qrc
FORMS += textfedit.ui \
screeninput.ui
