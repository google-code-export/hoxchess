#!/bin/bash

if [ "$1" == "clean" ]; then
    rm -f *.dylib
    cd ./AI_XQWLight && make -f Makefile.osx clean
    cd ../AI_HaQiKiD && make -f Makefile.osx clean
    cd ../AI_MaxQi && make -f Makefile.osx clean
    cd ../AI_Folium && make -f Makefile.osx clean
    cd ../AI_TSITO && make -f Makefile.osx clean
    exit 0
fi

cd ./AI_XQWLight && make -f Makefile.osx
cd ../AI_HaQiKiD && make -f Makefile.osx
cd ../AI_MaxQi && make -f Makefile.osx
cd ../AI_Folium && make -f Makefile.osx
cd ../AI_TSITO && make -f Makefile.osx
