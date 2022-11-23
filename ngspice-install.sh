#!/bin/bash
TARGET_DIR=`pwd`/build_ngspice
TARGET_FILE=`pwd`/build_ngspice/lib/libngspice.a

if test -f "$TARGET_FILE"; then
    echo "$FILE exists."
else
    echo "Entering ngspice source folder..."
    cd ./src/libraries/ngspice-36/

    #run autogen to generate configure
    echo "Run autogen..."
    ./autogen.sh

    #make relase directory
    mkdir release

    echo "Entering release folder..."
    cd release

    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "Compile linux shared..."
        # Linux
        make clean
        ../configure  --with-ngshared --with-readline=no --disable-debug --prefix=${TARGET_DIR}
        make
        make install    
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "Compile macos shared..."
        # Mac OSX
        make clean
        ../configure  --with-ngshared --with-readline=no --disable-debug --prefix=${TARGET_DIR}
        make
        make install
    fi
fi
