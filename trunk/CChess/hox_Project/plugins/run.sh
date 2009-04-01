#!/bin/sh

if [ "$1" == "clean" ]; then
    rm -f *.so
    cd ./AI_XQWLight && make clean
    cd ../AI_MaxQi && make clean
    cd ../AI_Folium && make clean
    cd ../AI_TSITO && make clean
    exit 0
fi

cd ./AI_XQWLight && make
cd ../AI_MaxQi && make
cd ../AI_Folium && make
cd ../AI_TSITO && make
