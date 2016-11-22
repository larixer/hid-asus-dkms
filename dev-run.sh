#! /bin/sh
make -C /lib/modules/$(uname -r)/build M=$PWD/src clean
make -C /lib/modules/$(uname -r)/build M=$PWD/src hid-asus-fte.ko
sudo rmmod hid_asus_fte
sudo insmod src/hid-asus-fte.ko
./dev-attach.sh
