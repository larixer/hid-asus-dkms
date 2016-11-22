#! /bin/sh
make -C /lib/modules/$(uname -r)/build M=$PWD/src clean
make -C /lib/modules/$(uname -r)/build M=$PWD/src hid-asus.ko
sudo rmmod hid_asus
sudo insmod src/hid-asus.ko
./dev-attach.sh
