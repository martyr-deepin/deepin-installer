# Deepin Installer

Deepin Installer, new installer for Deepin.

## Dependencies

### Build dependencies

* cmake
* coffeescript, front end is written in coffeescript.
* dde-go-dbus-factory
* deepin-webkit3, customized webkit-gtk3.
* libparted, backend of partition operation.
* sqlite3

### Runtime dependencies

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

## Build and Install

The easiest way to build deepin-installer is by using `debuild` command
which will generate a deb package.

If you need to build manually, run commands below:
```
mkdir build
cd build
cmake ..
make
```

And then type `$ sudo make install` to install it into system.

## TODO

* os-prober does not support EFI system.
* Update EFI environment when it is overwritten.

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](http://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for users](http://wiki.deepin.org/index.php?title=Contribution_Guidelines_for_Users)
* [Contribution guide for developers](http://wiki.deepin.org/index.php?title=Contribution_Guidelines_for_Developers).

## License

Deepin Installer is released under [GPLv3](LICENSE).
