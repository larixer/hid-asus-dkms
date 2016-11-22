#! /bin/sh
sudo dkms remove asus-fte/1.0 --all
sudo dkms remove asus/1.0 --all
./dev-restore.sh
