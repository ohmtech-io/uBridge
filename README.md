# ubridge


## Instalation instructions

sudo apt update
optional:
sudo apt upgrade

### Pre-requisites 

sudo apt install git cmake ninja-build


### GCC 10:

https://github.com/abhiTronix/raspberry-pi-cross-compilers/wiki/Native-Compiler:-Installation-Instructions

sudo apt-get install build-essential gawk gcc g++ gfortran git texinfo bison  wget bzip2 libncurses-dev libssl-dev openssl zlib1g-dev

wget https://sourceforge.net/projects/raspberry-pi-cross-compilers/files/Raspberry%20Pi%20GCC%20Native-Compiler%20Toolchains/Buster/GCC%2010.2.0/Raspberry%20Pi%203A%2B%2C%203B%2B%2C%204/native-gcc-10.2.0-pi_3%2B.tar.gz

tar xf native-gcc-10.2.0-pi_3%2B.tar.gz

#echo 'export PATH=/opt/gcc-10.1.0/bin:$PATH' >> ~/.bashrc
#echo 'export LD_LIBRARY_PATH=/opt/gcc-10.1.0/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
#. ~/.bashrc

sudo ln -s /usr/include/arm-linux-gnueabihf/sys /usr/include/sys
sudo ln -s /usr/include/arm-linux-gnueabihf/bits /usr/include/bits
sudo ln -s /usr/include/arm-linux-gnueabihf/gnu /usr/include/gnu
sudo ln -s /usr/include/arm-linux-gnueabihf/asm /usr/include/asm
sudo ln -s /usr/lib/arm-linux-gnueabihf/crti.o /usr/lib/crti.o
sudo ln -s /usr/lib/arm-linux-gnueabihf/crt1.o /usr/lib/crt1.o
sudo ln -s /usr/lib/arm-linux-gnueabihf/crtn.o /usr/lib/crtn


LD_LIBRARY_PATH=/home/pi/TEMP/native-pi-gcc-10.2.0-2/lib:$LD_LIBRARY_PATH
PATH=/home/pi/TEMP/native-pi-gcc-10.2.0-2/bin:$PATH
gcc --version

10.2.0:

cmake -GNinja -D CMAKE_C_COMPILER=gcc-10.2.0 -D CMAKE_CXX_COMPILER=g++-10.2.0 ..

#### Libserial

apt-cache policy libserial-dev

'''
libserial-dev:
  Installed: (none)
  Candidate: 0.6.0~rc2+svn122-4
  Version table:
     1.0.0-5 50
         50 http://archive.raspbian.org/raspbian testing/main armhf Packages
     0.6.0~rc2+svn122-4 500
        500 http://raspbian.raspberrypi.org/raspbian buster/main armhf Packages

sudo apt install libserial-dev=1.0.0-5

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
