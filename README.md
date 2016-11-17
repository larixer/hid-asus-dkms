## ASUS FTE1* TouchPads DKMS driver

## Getting Started

1. Clone source code locally.

  ```
  git clone git@github.com:vlasenko/hid-asus-fte-dkms.git
  cd hid-asus-fte-dkms
  ```

2. Install DKMS driver.

  ```
  ./dkms-add.sh
  ```

## Updating to the latest driver version

1. Pull latest source code from repository.
  ```
  cd hid-asus-fte-dkms
  git pull
  ```

2. Reinstall DKMS driver.

  ```
  ./dkms-add.sh
  ```

## Development Scripts

1. Script to clean compile the module from soruce without installing to dkms
  ```
  ./dev-run.sh
  ```
2. Script to restore back touchpad handling by i2c_hid driver
  ```
  ./dev-restore.sh
  ```
3. Script to simulate how the driver will behave on boot by removing and then adding the driver with modprobe
  ```
  ./dev-modprobe.sh
  ```
