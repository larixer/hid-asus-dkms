#! /bin/sh
sudo dkms remove asus-fte/1.0 --all
sudo dkms add ../hid-asus-fte-dkms
sudo dkms install -m asus-fte -v 1.0
./dev-attach.sh
