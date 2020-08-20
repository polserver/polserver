#!/bin/bash

# Script for setting up a development environment
# for POL on Linux machines (debian-based)

# Updates the system and install needed packages
# uses non-interactive mode (http://askubuntu.com/questions/146921/how-do-i-apt-get-y-dist-upgrade-without-a-grub-config-prompt)

echo
echo -------------------------------------------------------------------------------
echo apt-get update
echo -------------------------------------------------------------------------------
sudo DEBIAN_FRONTEND=noninteractive apt-get update

echo
echo -------------------------------------------------------------------------------
echo apt-get upgrade
echo -------------------------------------------------------------------------------
sudo DEBIAN_FRONTEND=noninteractive apt-get -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" upgrade

echo
echo -------------------------------------------------------------------------------
echo Compiler support and libraries
echo -------------------------------------------------------------------------------
echo
sudo apt-get install g++-9 g++-multilib zlib1g-dev gcc-9-locales gcc-9-doc libstdc++6-9-dbg -y
sudo apt-get install g++ -y

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
echo install clang
echo -------------------------------------------------------------------------------
echo
sudo apt-get install clang-7 libclang-7-dev libllvm-7-ocaml-dev lldb-7 llvm-7-examples clang-tidy-7 clang-format-7 -y
sudo apt-get install clang -y
echo

echo
echo -------------------------------------------------------------------------------
echo install cmake
echo -------------------------------------------------------------------------------
echo
sudo apt-get install cmake -y

echo
echo -------------------------------------------------------------------------------
echo needed for antlr
echo -------------------------------------------------------------------------------
echo
sudo apt-get install pkgconf uuid-dev -y

echo
echo -------------------------------------------------------------------------------
echo ninja
echo -------------------------------------------------------------------------------
sudo apt-get install ninja-build -y

echo
echo -------------------------------------------------------------------------------
echo formatting
echo -------------------------------------------------------------------------------
sudo apt-get install clang-format -y

echo
echo -------------------------------------------------------------------------------
echo debugging
echo -------------------------------------------------------------------------------
sudo apt-get install gdb valgrind -y

# Creates a link to /vagrant (which, if you run this file, will contain the git repository)
if [ ! -d "/home/vagrant/polserver" ]; then
	ln -s /vagrant /home/vagrant/polserver
fi
