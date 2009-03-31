SOURCES += main.cpp \
 welcome.cpp \
 expert.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
TARGET = nixstbuild
DESTDIR = ../bin
HEADERS += welcome.h \
 expert.h

