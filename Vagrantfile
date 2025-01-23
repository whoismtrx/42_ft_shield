Vagrant.configure("2") do |config|
	config.vm.box = "debian/bullseye64"
	config.vm.provider "virtualbox" do |vb|
		vb.memory = "1024"
		vb.name = "ft_shield_vm"
	end
	config.vm.network "private_network", ip: "192.168.56.20"
	config.vm.synced_folder "/Users/orekabe/Desktop/42_ft_shield", "/home/vagrant/ft_shield"
	config.vm.provision "shell", inline: <<-SHELL
		apt-get update -qq
		apt-get install -y build-essential gdb
		chown -R vagrant:vagrant "/home/vagrant/ft_shield"
	SHELL
end