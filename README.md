# ubridge
__uBridge__ is a lightweight, modular application that can log data into CSV files, publish into InfluxDB databases and MQTT servers.

![uBridge](/img/uBridge-diagram.svg)

More info on: https://docs.ohmtech.io/ubridge/intro

## Instalation instructions
```bash
sudo apt update
```
optional:
```bash
sudo apt upgrade
```

### Pre-requisites 
```bash
sudo apt install git cmake ninja-build
```

<!-- sudo apt-get install build-essential gawk gcc g++ gfortran git texinfo bison  wget bzip2 libncurses-dev libssl-dev openssl zlib1g-dev -->


### Libserial

```bash
sudo apt install libserial-dev
```


### NNG

```bash
cd /tmp
git clone https://github.com/nanomsg/nng.git
cd nng
git checkout 9d6b241
mkdir build
cd build

cmake -G Ninja ..
ninja
sudo ninja install
```
## Building ubridge

```bash
cd /tmp
git clone https://github.com/ohmtech-io/uBridge.git
cd uBridge
mkdir build 
cd build && cmake ..
make
```

## Install service

```bash
sudo make install
sudo systemctl enable ubridge ubridge-server
```

## Start the service
``` bash
sudo systemctl start ubridge
```

## Monitoring
```bash
 systemctl status ubridge-server.service
 ```

### Log file
```bash
tail /tmp/ubridge.log
```
