SOURCES += main.cpp \
 welcome.cpp \
 expert.cpp \
 utils.cpp \
 luaparser.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
TARGET = nixstbuild
DESTDIR = ../bin
HEADERS += welcome.h \
 expert.h \
 utils.h \
 luaparser.h


LIBS += -L../../shared/lib/linux/x86/ \
  -L../../shared/lib/linux/x86_64/ \
  -lmain \
  -lelf \
  -llua

DISTFILES += lua/nixstbuild.lua \
 ../TODO \
 lua/utils-public.lua

INCLUDEPATH += ../../shared/include/ \
  ../../shared/

