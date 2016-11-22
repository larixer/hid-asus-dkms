#! /bin/sh
DEV_NO=`dmesg | grep "on i2c-FTE100" | sed -n 's/[^a-z]*[^0-9]*\([0-9A-F\.:]*\):.*/\1/p' | tail -1`
sudo modprobe hid_asus
if [ ! -h /sys/bus/hid/drivers/hid-asus/$DEV_NO ]; then
	echo Rebinding $DEV_NO to hid-asus
	sudo sh -c "echo $DEV_NO > /sys/bus/hid/drivers/hid-generic/unbind"
	sudo sh -c "echo $DEV_NO > /sys/bus/hid/drivers/hid-asus/bind"
fi