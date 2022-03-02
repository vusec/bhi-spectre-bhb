#!/bin/sh
sudo mkdir -p /mnt/chroot
sudo mount -o loop stretch.img /mnt/chroot

sudo mkdir -p /mnt/chroot/root/bhi_test
sudo mkdir -p /mnt/chroot/root/fr_checker

sudo cp ../../bhi_test/main.o                   /mnt/chroot/root/bhi_test/main.o
sudo cp ../../bhi_test/run.sh                   /mnt/chroot/root/bhi_test/run.sh

sudo cp ../../../../tools/fr_checker/main.o     /mnt/chroot/root/fr_checker/main.o
sudo cp ../../../../tools/fr_checker/run.sh     /mnt/chroot/root/fr_checker/run.sh

sudo umount /mnt/chroot
