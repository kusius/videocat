#!/bin/sh
PROGRAM_NAME=videocat
CC=clang

cd ./build

$CC -g -I../src/thirdparty/libmisb/include ../src/main.c ../src/thirdparty/libmisb/src/*.c -lavformat -lavcodec -lavdevice -lavfilter -lavresample -lavutil -lswscale -lz -lbz2 \
-framework CoreFoundation -framework CoreVideo -framework VideoDecodeAcceleration \
-o $PROGRAM_NAME