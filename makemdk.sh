#!/bin/bash
touch -a mdktest.cpp

# download MDK for your platform from:
# https://sourceforge.net/projects/mdk-sdk/files/
# copy the include folder here and libs to /usr/local/lib
# make sure you have sfml libs there and ffmpeg libs ( can be just sudo apt-get install ffmepg )
# run this script, ignore the flood of "invalid string offset"
# copy mdktest binary to the folder with your mp4 files

# space - play/pause
# left/right - prev/next file

g++ -Wno-psabi -c mdktest.cpp -Iinclude/
g++ -o mdktest -lsfml-graphics -lsfml-window -lsfml-system -lmdk -lgbm mdktest.o -Llib/

