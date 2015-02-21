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
sudo apt-get install subversion -y

echo
echo -------------------------------------------------------------------------------
echo MySQL
echo -------------------------------------------------------------------------------
echo
echo sudo apt-get install mysql-client libmysqlclient-dev -y

echo
echo -------------------------------------------------------------------------------
echo  add-apt-repository
echo -------------------------------------------------------------------------------
echo
sudo apt-get install python-software-properties -y

echo
echo -------------------------------------------------------------------------------
echo add gcc 4.8 repro
echo -------------------------------------------------------------------------------
echo
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y

echo
echo -------------------------------------------------------------------------------
echo add llvm repro
echo -------------------------------------------------------------------------------
echo
sudo add-apt-repository 'deb http://llvm.org/apt/precise/ llvm-toolchain-precise main'
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
sudo apt-get install clang-3.7 clang-3.7-doc libclang-common-3.7-dev libclang-3.7-dev libclang1-3.7 libllvm-3.7-ocaml-dev libllvm3.7 lldb-3.7 llvm-3.7 llvm-3.7-dev llvm-3.7-doc llvm-3.7-examples llvm-3.7-runtime clang-modernize-3.7 clang-format-3.7 -y
# libclang1-3.4-dbg
# libllvm3.4-dbg

echo
echo -------------------------------------------------------------------------------
echo install gcc 4.8
echo -------------------------------------------------------------------------------
echo
echo sudo apt-get install g++-4.8 cpp-4.8 gcc-4.8 libcloog-isl4 libisl10 gcc-4.8-locales g++-4.8-multilib gcc-4.8-doc libstdc++6-4.8-dbg gcc-4.8-multilib libmudflap0-4.8-dev libgomp1-dbg libitm1-dbg libatomic1-dbg libasan0-dbg libtsan0-dbg libquadmath0-dbg libmudflap0-dbg binutils-gold -y




# Check out the code (will take a while because of boost)
# svn checkout svn://svn.code.sf.net/p/polserver/code/trunk polserver

# Creates a link to /vagrant (which, if you run this file, will contain the SVN repository)
if [ ! -d "/home/vagrant/polserver" ]; then
	ln -s /vagrant /home/vagrant/polserver
fi