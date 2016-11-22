#! /bin/sh
sudo dkms remove asus-fte/1.0 --all
sudo dkms remove asus/1.0 --all
sudo dkms add ../hid-asus-dkms
sudo dkms install -m asus -v 1.0
./dev-attach.sh
