#!/bin/sh
#< build-me.sh - 20150407 - build gshhg project
BN=`basename $0`
TMPLOG="bldlog-1.txt"
TMPSRC=".."

if [ ! -f "$TMPSRC/CMakeLists.txt" ]; then
    echo "ERROR: Can NOT locate $TMPSRC/CMakeLists.txt! WHERE IS IT! FIX ME!!!"
    exit 1
fi

# NOTE: install location - will be in $HOME/bin, which should be in your PATH
TMPOPTS="-DCMAKE_INSTALL_PREFIX:PATH=$HOME"
# TMPOPTS="-DCMAKE_INSTALL_PREFIX:PATH=$HOME/projects/install/magvar"

wait_for_input()
{
    if [ "$#" -gt "0" ] ; then
        echo "$1"
    fi
    echo -n "Enter y to continue : "
    read char
    if [ "$char" = "y" -o "$char" = "Y" ]
    then
        echo "Got $char ... continuing ..."
    else
        if [ "$char" = "" ] ; then
            echo "Aborting ... no input!"
        else
            echo "Aborting ... got $char!"
        fi
        exit 1
    fi
}

ask()
{
    wait_for_input "$BN: *** CONTINUE? ***"
}

VERBOSE=0
DBGSYMS=0
DOPAUSE=0

# option like EXTRA=-DCMAKE_BUILD_TYPE=DEBUG
add_extra_cmopt()
{   
    if [ "$#" -gt "0" ]; then
        LEN1=`expr length $1`
        if [ "$LEN1" -gt "7" ]; then
            XOPT=`echo $1 | cut -b7-$LEN1`
            TMPOPTS="$TMPOPTS $XOPT"
            echo "$BN: Add EXTRA option [$XOPT]"
        else
            echo "$BN: ERROR: Length $LEN! less than/equals 6"
            exit 1
        fi
    else
        echo "$BN: ERROR: No option passed!"
        exit 1
    fi
}

give_help()
{
    echo "$BN [OPTIONS]"
    echo "OPTIONS"
    echo " VERBOSE = Use verbose build (def=$VERBOSE)"
    echo " DEBUG   = Enable DEBUG symbols (-g)."
    echo " EXTRA=CMOPT = Add a extra CMake option."
#    echo " NOPAUSE = Skip the pausing before each step."
    #echo " PROFILING = Enable PROFILING (-pg)."
    echo ""
    exit 1
}


for arg in $@; do
      case $arg in
         VERBOSE) VERBOSE=1 ;;
         DEBUG) DBGSYMS=1 ;;
#         NOPAUSE) DOPAUSE=0 ;;
         EXTRA=*) add_extra_cmopt $arg ;;
         --help) give_help ;;
         -h) give_help ;;
         -\?) give_help ;;
         *)
            echo "$BN: ERROR: Invalid argument [$arg]"
            exit 1
            ;;
      esac
done

if [ "$VERBOSE" = "1" ]; then
    TMPOPTS="$TMPOPTS -DCMAKE_VERBOSE_MAKEFILE=TRUE"
    echo "$BN: Enabling VERBOSE make"
fi

if [ "$DBGSYMS" = "1" ]; then
    TMPOPTS="$TMPOPTS -DCMAKE_BUILD_TYPE=Debug -DENABLE_DEBUG_SYMBOLS:BOOL=TRUE"
    echo "$BN: Enabling DEBUG symbols"
fi

echo "$BN: Will do 'cmake $TMPSRC $TMPOPTS'"
if [ "$DOPAUSE" = "1" ]; then
    ask
    echo "$BN: Doing 'cmake $TMPSRC $TMPOPTS'"
fi

echo $TMPSRC $DATE $TIME > $TMPLOG
echo "$BN: Doing 'cmake $TMPSRC $TMPOPTS', output to $TMPLOG"
cmake $TMPSRC $TMPOPTS >> $TMPLOG 2>&1
if [ ! "$?" = "0" ]; then
    tail $TMPLOG
    echo "$BN: cmake config gen ERROR! see $TMPLOG for details"
    exit 1
fi

# maybe not required
#if [ "$DBGSYMS" = "1" ]; then
#    echo "cmake --build . --config Debug"
#    cmake --build . --config Debug >> $TMPLOG 2>&1
#else
    echo "$BN: Doing 'make', output to $TMPLOG"
    make >> $TMPLOG 2>&1
#fi
if [ ! "$?" = "0" ]; then
    tail $TMPLOG
    echo "$BN: make ERROR! see $TMPLOG for details..."
    exit 1
fi

echo "$BN: Appear a successful build..."  >> $TMPLOG 2>&1
tail $TMPLOG
### echo "Appear successful..."
echo "$BN: Time for 'make install' if desired... to $HOME/bin unless changed..."
exit 0

# @REM eof

