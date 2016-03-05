What is this about
============

We are using Vagrant to set up a standard Linux development virtual
machine for POL. It runs Ubuntu 12.04 LTS.


Installing Vagrant
============

Read the docs here: 
http://docs.vagrantup.com/v2/installation/index.html

The most recent version of Vagrant can be found at: 
http://downloads.vagrantup.com/


Running the machine
=============

After installing, go to the root folder (where the "Vagrant" file is) and type:

	vagrant up

It will take a while to create the virtual machine and install the required packages.
Once this is done, you can access the machine by typing:
	
	vagrant ssh

If you are on Windows (and not using cygwin or git-bash), there will be an error. The error
should have enough information so you can connect using Putty or a similar SSH client.


