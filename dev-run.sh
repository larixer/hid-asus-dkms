#! /bin/sh
make -C /lib/modules/$(uname -r)/build M=$PWD/src clean
make -C /lib/modules/$(uname -r)/build M=$PWD/src hid-asus-fte.ko
sudo /sbin/rmmod hid_asus_fte i2c_hid hid_generic
sudo insmod src/hid-asus-fte.ko 
sudo modprobe i2c_hid
