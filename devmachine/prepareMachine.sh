#!/bin/bash

# Script for setting up a development environment
# for POL on Linux machines (debian-based)

# Updates the system and install needed packages
# uses non-interactive mode (http://askubuntu.com/questions/146921/how-do-i-apt-get-y-dist-upgrade-without-a-grub-config-prompt)
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" upgrade

# Compiler support and libraries
sudo apt-get install make g++ libc-dev g++-multilib libgomp1 --install-suggests -y

# Version control
sudo apt-get install subversion -y

# Check out the code (will take a while because of boost)
# svn checkout svn://svn.code.sf.net/p/polserver/code/trunk polserver

# Creates a link to /vagrant (which, if you run this file, will contain the SVN repository)
if [ ! -d "/home/vagrant/polserver" ]; then
	ln -s /vagrant /home/vagrant/polserver
fi