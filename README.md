## ASUS HID DKMS driver

## Prerequisites

1. Ensure secure boot is not enabled. If you have secure boot enabled, 
after installing this DKMS driver you will get this error message in kernel log:

  ```
  modprobe: ERROR: could not insert 'hid_asus': Required key not available
  ```

You won't be able to run third-party (aka 'out of tree') kernel modules with 
secure boot. This is because for secure boot to work all the modules 
have to be signed - and given our module is third party it hasn't been signed.

If you want to use this module you'll need to disable secure boot.

## Getting Started

1. Clone source code locally.

  ```
  git clone https://github.com/vlasenko/hid-asus-dkms.git
  cd hid-asus-dkms
  ```

2. Install DKMS driver and load kernel module into memory.

  ```
  ./dkms-add.sh
  ```

Check that multi-touch gestures work after executing this command
and after reboot as well.

## Submitting issues

Please use GitHub issue tracker for submitting issues with DKMS driver:

https://github.com/vlasenko/hid-asus-dkms/issues

This is the most convenient place for us to provide support on this project.

## Updating to the latest driver version

1. Pull latest source code from repository.
  ```
  cd hid-asus-dkms
  git pull
  ```

2. Reinstall DKMS driver.

  ```
  ./dkms-add.sh
  ```

This will install DKMS driver into the system and reloads it immediately.

## Development Scripts

1. Script to clean compile the module from soruce without installing to dkms
  ```
  ./dev-run.sh
  ```
2. Script to bind touchpad to this DKMS driver, until reboot
  ```
  ./dev-attach.sh
  ```
3. Script to restore back touchpad handling by hid_generic driver, until reboot
  ```
  ./dev-restore.sh
  ```
