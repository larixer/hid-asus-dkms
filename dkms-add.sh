#! /bin/sh
sudo dkms remove asus-fte/1.0 --all 2>/dev/null
sudo dkms remove asus/1.0 --all 2>/dev/null
sudo dkms add .
sudo dkms install -m asus -v 1.0
./dev-attach.sh
[ -d /etc/udev/rules.d ] && echo 'ACTION=="add" ENV{MODALIAS}=="hid:b0018g0001v00000B05p00000101", RUN+="/bin/sh -c '\''kname=`uname -r`; insmod `find /lib/modules/$kname -name hid-asus.ko ! -wholename /lib/modules/$kname/kernel/drivers/hid/hid-asus.ko`; D=`basename %p`; echo $D > /sys/bus/hid/drivers/hid-generic/unbind; echo $D > /sys/bus/hid/drivers/hid-asus/bind'\'\" | sudo bash -c 'cat > /etc/udev/rules.d/41-hid-asus.rules'
