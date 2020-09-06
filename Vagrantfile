# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "ubuntu/focal64"

  config.vm.provision :shell, :path => "./devmachine/prepareMachine.sh"
  config.vm.boot_timeout = 600

  # Forwards port 5003 from the VM to port 5004 at host (useful for testing)
  config.vm.network :forwarded_port, guest: 5003, host: 5004
  
  config.vm.provider "virtualbox" do |v|
    v.memory = 3024
    v.cpus = 2
    v.customize ["setextradata", :id, "VBoxInternal2/SharedFoldersEnableSymlinksCreate/v-root", "1"]
	
  end
end
