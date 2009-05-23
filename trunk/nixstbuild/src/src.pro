SOURCES += main.cpp \
 welcome.cpp \
 expert.cpp \
 utils.cpp \
 luaparser.cpp \
 configw.cpp \
 editor.cpp \
 editsettings.cpp \
 rungen.cpp \
 instscreenwidget.cpp \
 newinstscreen.cpp \
 treeedit.cpp \
 newinstwidget.cpp \
 deskentrywidget.cpp \
 morelabel.cpp \
 dirbrowser.cpp \
 dirinput.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 debug
DESTDIR = ../bin
HEADERS += welcome.h \
 expert.h \
 utils.h \
 luaparser.h \
 configw.h \
 editor.h \
 editsettings.h \
 rungen.h \
 instscreenwidget.h \
 newinstscreen.h \
 main.h \
 treeedit.h \
 newinstwidget.h \
 deskentrywidget.h \
 morelabel.h \
 dirbrowser.h \
 dirinput.h



DISTFILES += lua/nixstbuild.lua \
 ../TODO \
 lua/utils-public.lua \
 lua/addwidget.lua \
 lua/genrun.lua \
 lua/filemanager.lua

TARGETDEPS += ../qcodeedit/libqcodeedit.so
LIBS += -L../qcodeedit \
  -lqcodeedit \
  -L../../shared/lib/linux/x86/ \
  -L../../shared/lib/linux/x86_64/ \
  -lmain \
  -lelf \
  -llua

TARGET = nixstbuild


RESOURCES += editor.qrc \
 nixstbuild.qrc

INCLUDEPATH += ../qcodeedit/lib \
  ../qcodeedit/lib/widgets/ \
  ../qcodeedit/lib/document/ \
  ../../shared/include/ \
  ../../shared/

CONFIG -= release

