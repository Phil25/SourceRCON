#!/bin/bash

./compile.sh

PROJECT_NAME=`basename $(pwd)`

if [ -f ./bin/${PROJECT_NAME} ]; then
	./bin/${PROJECT_NAME}
fi
