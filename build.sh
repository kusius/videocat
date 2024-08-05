#!/bin/sh
PROGRAM_NAME=videocat
CC=clang

cd ./build

$CC -g ../src/main.c  -lavformat -lavcodec -lavdevice -lavfilter -lavresample -lavutil -lswscale -lz -lbz2 \
-framework CoreFoundation -framework CoreVideo -framework VideoDecodeAcceleration \
-o $PROGRAM_NAME