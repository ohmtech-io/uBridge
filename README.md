# ubridge

## Instalation instructions
```bash
sudo apt update
```
optional:
```bash
sudo apt upgrade
```

### Pre-requisites 

sudo apt install git cmake ninja-build


<!-- sudo apt-get install build-essential gawk gcc g++ gfortran git texinfo bison  wget bzip2 libncurses-dev libssl-dev openssl zlib1g-dev -->


### Libserial

```bash
sudo apt install g++ git autogen autoconf build-essential cmake graphviz \
                 libboost-dev libboost-test-dev libgtest-dev libtool \
                 python3-sip-dev doxygen python3-sphinx pkg-config \
                 python3-sphinx-rtd-theme

git clone https://github.com/crayzeewulf/libserial.git
cd libserial
git checkout 1d1e47a
./compile.sh
cd build
sudo make install
sudo ldconfig
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
mkdir build 
cd build && cmake ..
make
```

## Installi service

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
