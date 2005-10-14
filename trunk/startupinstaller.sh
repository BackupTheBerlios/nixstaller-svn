#/bin/sh

# Does some simple checking, and tries to launch the right installer frontend

OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`

# Only ncurses for now...
RUNCOMMAND="./frontends/$CURRENT_OS/fltk $HOME"

#Incase there are missing libraries, we just the lib path...
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:./lib/$CURRENT_OS" $RUNCOMMAND
