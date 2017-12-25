#!/bin/bash

PROJECT_NAME=`basename $(pwd)`
rm -f bin/${PROJECT_NAME}
echo Compiling $PROJECT_NAME...

g++ `ls -d $PWD/src/*` -o bin/${PROJECT_NAME} -static-libgcc -static-libstdc++ -std=c++17
