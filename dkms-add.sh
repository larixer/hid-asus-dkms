#! /bin/sh
sudo dkms remove asus-fte/1.0 --all 2>/dev/null
sudo dkms remove asus/1.0 --all 2>/dev/null
sudo dkms add .
sudo dkms install -m asus -v 1.0
./dev-attach.sh
