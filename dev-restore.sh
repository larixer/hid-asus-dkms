#! /bin/sh
DEV_NO=`dmesg | grep "on i2c-FTE100" | sed -n 's/[^a-z]*[^0-9]*\([0-9A-F\.:]*\):.*/\1/p' | tail -1`
sudo rmmod hid_asus
if [ ! -h /sys/bus/hid/drivers/hid-generic/$DEV_NO ]; then
	echo Rebinding $DEV_NO to hid-generic
	sudo sh -c "echo $DEV_NO > /sys/bus/hid/drivers/hid-generic/bind"
	sudo rmmod i2c_hid
	sudo modprobe i2c_hid
fi