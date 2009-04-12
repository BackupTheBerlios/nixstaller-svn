SOURCES += main.cpp \
 welcome.cpp \
 expert.cpp \
 utils.cpp \
 luaparser.cpp \
 configw.cpp \
 editor.cpp \
 editsettings.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
DESTDIR = ../bin
HEADERS += welcome.h \
 expert.h \
 utils.h \
 luaparser.h \
 configw.h \
 editor.h \
 editsettings.h



DISTFILES += lua/nixstbuild.lua \
 ../TODO \
 lua/utils-public.lua

TARGETDEPS += ../qcodeedit/libqcodeedit.so
LIBS += -L../qcodeedit \
  -lqcodeedit \
  -L../../shared/lib/linux/x86/ \
  -L../../shared/lib/linux/x86_64/ \
  -lmain \
  -lelf \
  -llua

TARGET = nixstbuild

INCLUDEPATH += ../qcodeedit/lib \
  ../qcodeedit/lib/document/ \
  ../../shared/include/ \
  ../../shared/

RESOURCES += editor.qrc

