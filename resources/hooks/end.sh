#!/bin/bash
hookspath=/usr/share/installer/resources/hooks

echo test deepin-installer

echo cd $hookspath/
cd $hookspath/

echo touch /tmp/test-deepin-install-ok.txt
touch /tmp/test-deepin-install-ok.txt

echo ./55-netcfg-network-manager
./55-netcfg-network-manager

echo ./56enable-network-manager
./56enable-network-manager

echo ./70remove-overlaymirror.sh
./70remove-overlaymirror.sh

exit 0
