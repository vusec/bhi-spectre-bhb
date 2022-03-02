#!/bin/bash
if [ "$EUID" -ne 0 ]
    then echo "Please run as root for /proc/pid/pagemap"
    exit
fi

taskset -c 0 ./main.o
