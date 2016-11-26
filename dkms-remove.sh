#! /bin/sh
sudo dkms remove asus-fte/1.0 --all 2>/dev/null
sudo dkms remove asus/1.0 --all 2>/dev/null
udevrule=/etc/udev/rules.d/41-hid-asus.rules
[ -f $udevrule ] && sudo rm $udevrule
./dev-restore.sh
