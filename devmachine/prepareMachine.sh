#!/bin/bash

# Script for setting up a development environment
# for POL on Linux machines (debian-based)

# Updates the system and install needed packages
# uses non-interactive mode (http://askubuntu.com/questions/146921/how-do-i-apt-get-y-dist-upgrade-without-a-grub-config-prompt)
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" upgrade

echo
echo -------------------------------------------------------------------------------
echo Compiler support and libraries
echo -------------------------------------------------------------------------------
echo
sudo apt-get install make g++ libc-dev g++-multilib libgomp1 --install-suggests -y

echo
echo -------------------------------------------------------------------------------
echo VIM
echo -------------------------------------------------------------------------------
echo
sudo apt-get install vim -y

echo
echo -------------------------------------------------------------------------------
echo Version control
echo -------------------------------------------------------------------------------
echo
sudo apt-get install git -y

echo
echo -------------------------------------------------------------------------------
echo MySQL
echo -------------------------------------------------------------------------------
echo
sudo apt-get install mysql-client libmysqlclient-dev -y


echo
echo -------------------------------------------------------------------------------
echo OpenSSL
echo -------------------------------------------------------------------------------
echo
sudo apt-get install libssl-dev -y

echo
echo -------------------------------------------------------------------------------
echo  add-apt-repository
echo -------------------------------------------------------------------------------
echo
sudo apt-get install python-software-properties -y

echo
echo -------------------------------------------------------------------------------
echo add gcc repro
echo -------------------------------------------------------------------------------
echo
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y

echo
echo -------------------------------------------------------------------------------
echo add llvm repro
echo -------------------------------------------------------------------------------
echo
sudo add-apt-repository 'deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.7 main'
sudo wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -

echo
echo -------------------------------------------------------------------------------
echo update
echo -------------------------------------------------------------------------------
echo
sudo apt-get update

echo
echo -------------------------------------------------------------------------------
echo install clang
echo -------------------------------------------------------------------------------
echo
sudo apt-get install clang-3.7 clang-3.7-doc libclang-common-3.7-dev libclang-3.7-dev libclang1-3.7 libllvm-3.7-ocaml-dev libllvm3.7 lldb-3.7 llvm-3.7 llvm-3.7-dev llvm-3.7-doc llvm-3.7-examples llvm-3.7-runtime clang-tidy-3.7 clang-format-3.7 llvm-toolchain-3.7 -y
#sudo apt-get install libiomp5 -y
echo
echo -------------------------------------------------------------------------------
echo install gcc 5
echo -------------------------------------------------------------------------------
echo
sudo apt-get install g++-5 cpp-5 gcc-5  gcc-5-locales g++-5-multilib gcc-5-doc libstdc++6-5-dbg gcc-5-multilib libcloog-isl4 libisl10 libgomp1-dbg libitm1-dbg libatomic1-dbg libasan0-dbg libtsan0-dbg libquadmath0-dbg binutils -y

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 1 --force
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 1 --force

echo
echo -------------------------------------------------------------------------------
echo install cmake
echo -------------------------------------------------------------------------------
echo
wget http://www.cmake.org/files/v3.17/cmake-3.17.1.tar.gz
tar -xvzf cmake-3.17.1.tar.gz
(cd cmake-3.17.1 && ./configure && make && sudo make install)
rm -rf cmake-3.17.1 cmake-3.17.1.tar.gz

echo
echo -------------------------------------------------------------------------------
echo needed for antlr
echo -------------------------------------------------------------------------------
echo
sudo apt-get install pkg-config uuid-dev


sudo apt-get upgrade

# Creates a link to /vagrant (which, if you run this file, will contain the SVN repository)
if [ ! -d "/home/vagrant/polserver" ]; then
	ln -s /vagrant /home/vagrant/polserver
fi

