## ASUS HID DKMS driver

## Getting Started

1. Clone source code locally.

  ```
  git clone https://github.com/vlasenko/hid-asus-dkms.git
  cd hid-asus-dkms
  ```

2. Build and load the kernel module into memory to check
   that it's working for you.
  ```
  ./dev-run.sh
  ```

3. Install DKMS driver, so that it loads on boot.

  ```
  ./dkms-add.sh
  ```

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
