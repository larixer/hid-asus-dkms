#! /bin/sh
sudo dkms remove asus-fte/1.0 --all
sudo dkms add ../hid-asus-fte-dkms
sudo dkms install -m asus-fte -v 1.0
sudo modprobe -r hid_asus_fte
sudo modprobe hid_asus_fte
