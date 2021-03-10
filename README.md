# ubridge

## Instalation instructions

sudo apt update
optional:
sudo apt upgrade

### Pre-requisites 

sudo apt install git cmake ninja-build


<!-- sudo apt-get install build-essential gawk gcc g++ gfortran git texinfo bison  wget bzip2 libncurses-dev libssl-dev openssl zlib1g-dev -->


#### Libserial

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


### NNG
  $ cd /tmp
  $ git clone https://github.com/nanomsg/nng.git
  $ cd nng
  $ git checkout 9d6b241
  $ mkdir build
  $ cd build
  
  $ cmake -G Ninja ..
  $ ninja
  $ ninja test #The following tests FAILED:
				 #30 - nng.sp.transport.tcp.tcp_test (Failed)
				 #but it doesn't seem to create an issue for ubridge
  $ sudo ninja install

## Building

mkdir build 
cd build && cmake -GNinja ..

or just:
cmake ..
make