#!/bin/bash -x
PLATFORM=$1
INPUT=$3
ANSWER=$2

if [ $INPUT == "stdin" ]; then
    INPUT=/dev/stdin
fi 

if [ $PLATFORM == "arm" ]; then
    CC=arm-linux-gnueabihf-gcc
    RUNNER=qemu-arm-static
elif [ $PLATFORM == "riscv" ]; then
    CC=
    RUNNER=
fi

$CC output.s libsysy.a -static -o output.$PLATFORM
$RUNNER output.$PLATFORM < $INPUT > output.$PLATFORM.out
echo -e "\n$?" >> output.$PLATFORM.out
