# About
Deepin Installer, new installer for linuxdeepin(TM).

# Dependencies
* coffeescript, front end is written in coffeescript.
* deepin-webkit3, customized webkit-gtk3.
* libparted, backend of partition operation.
* dde-go-dbus-factory
* btrfs-tools
* dosfstools
* e2fsprogs
* jfsutils
* unionfs-fuse
* ntfs-3g
* xfsprogs
* hfsprogs
* reiserfsprogs
* squashfs-tools
* os-prober

# Build and Install
Easist way to build deepin-installer is by using `debuild` command.

If you need to build manually, execute commands below:
```
mkdir build
cd build
cmake ..
make
```

And then type `$ sudo make isntall` to install it into system.

# Getting involved
We encourage you to report issues and contribute changes. Please check out the [Contribution Guidelines](http://wiki.deepin.org/index.php?title=Contribution_Guidelines) about how to proceed.

# TODO
* os-prober does not support EFI system.
* Update EFI environment when it is overwritten.

# License
Deepin Installer is released under GPLv3.
