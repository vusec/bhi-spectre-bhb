#!/bin/sh
sudo adb push main.o /data/local/tmp
sudo adb shell "su -c 'cd /data/local/tmp; chmod +x main.o; taskset 10 ./main.o'"
