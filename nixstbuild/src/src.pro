SOURCES += nixstbuild.cpp \
           main.cpp \
           sdialog.cpp \
           rungen.cpp
HEADERS += nixstbuild.h \
sdialog.h \
rungen.h
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
TARGET = ../bin/nixstbuild
RESOURCES = application.qrc
FORMS += textfedit.ui \
screeninput.ui
