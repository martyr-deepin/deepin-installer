#!/bin/bash

echo testdeepin-installer
echo touch /tmp/testdeepin-install.txt
touch /tmp/testdeepin-install.txt
echo sudo cp /tmp/testdeepin-install.txt /usr/share/installer/resources/
sudo cp /tmp/testdeepin-install.txt /usr/share/installer/resources/

./55-netcfg-network-manager

./56enable-network-manager

./70remove-overlaymirror.sh

exit 0
