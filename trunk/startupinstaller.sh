#/bin/sh

# Does some simple checking, and tries to launch the right installer frontend

OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`

# X Running?
if [ -z $DISPLAY ]; then
    # Not running, use ncurses frontend
    RUNCOMMAND="./frontends/$CURRENT_OS/ncurs $HOME"
else
    RUNCOMMAND="./frontends/$CURRENT_OS/fltk $HOME"
fi

#Incase there are missing libraries, we just change the lib path...
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:./lib/$CURRENT_OS" $RUNCOMMAND
