#! /bin/sh
# Remove overlay-mirror in squashfs
if [ -d /opt/overlay-mirror ]; then
    rm -rf /opt/overlay-mirror
fi

exit 0
