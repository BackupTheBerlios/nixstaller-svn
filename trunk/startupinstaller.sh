#/bin/sh

# Does some simple checking, and tries to launch the right installer frontend

OS=`uname`
CURRENT_OS=`echo "$OS" | tr [:upper:] [:lower:]`

NCURS=
FLTK=
RUNCOMMAND=

# Check if ncurses frontend exists
if [ -e ./frontends/$CURRENT_OS/ncurs ]; then
    NCURS="./frontends/${CURRENT_OS}/ncurs"
fi

# Check if fltk frontend exists
if [ -e ./frontends/$CURRENT_OS/fltk ]; then
    FLTK="./frontends/${CURRENT_OS}/fltk"
fi

# Do both frontends exist?
if [ ! -z $NCURS -a ! -z $FLTK ]; then
    # X Running?
    if [ -z $DISPLAY ]; then
        RUNCOMMAND="${NCURS} $CURRENT_OS" # Not running, use ncurses frontend
    else
        RUNCOMMAND="$FLTK $CURRENT_OS"
    fi
elif [ ! -z $NCURS ]; then
    RUNCOMMAND="${NCURS} $CURRENT_OS"
elif [ ! -z $FLTK ]; then
    RUNCOMMAND="$FLTK $CURRENT_OS"
else
    echo "Error: Couldn't find any frontend to use!"
    exit 1
fi

#Incase there are missing libraries, we just change the lib path...
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:./lib/$CURRENT_OS" $RUNCOMMAND
