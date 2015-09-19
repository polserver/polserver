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
echo sudo apt-get install mysql-client libmysqlclient-dev -y


echo
echo -------------------------------------------------------------------------------
echo OpenSSL
echo -------------------------------------------------------------------------------
echo
echo sudo apt-get install libssl-dev -y

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
sudo add-apt-repository 'deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty main'
sudo wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -

echo
echo -------------------------------------------------------------------------------
echo add cmake repro
echo -------------------------------------------------------------------------------
echo
sudo add-apt-repository ppa:george-edison55/cmake-3.x -y

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
sudo apt-get install clang-3.8 clang-3.8-doc libclang-common-3.8-dev libclang-3.8-dev libclang1-3.8 libllvm-3.8-ocaml-dev libllvm3.8 lldb-3.8 llvm-3.8 llvm-3.8-dev llvm-3.8-doc llvm-3.8-examples llvm-3.8-runtime clang-modernize-3.8 clang-format-3.8 -y

echo
echo -------------------------------------------------------------------------------
echo install gcc 5
echo -------------------------------------------------------------------------------
echo
sudo apt-get install g++-5 cpp-5 gcc-5  gcc-5-locales g++-5-multilib gcc-5-doc libstdc++6-5-dbg gcc-5-multilib libcloog-isl4 libisl10 libgomp1-dbg libitm1-dbg libatomic1-dbg libasan0-dbg libtsan0-dbg libquadmath0-dbg binutils -y

echo
echo -------------------------------------------------------------------------------
echo install cmake
echo -------------------------------------------------------------------------------
echo
sudo apt-get install cmake -y

sudo apt-get upgrade

# Check out the code (will take a while because of boost)
# svn checkout svn://svn.code.sf.net/p/polserver/code/trunk polserver

# Creates a link to /vagrant (which, if you run this file, will contain the SVN repository)
if [ ! -d "/home/vagrant/polserver" ]; then
	ln -s /vagrant /home/vagrant/polserver
fi

