#!/bin/bash
HOOKSPATH=/usr/share/installer/resources/hooks

if [ -f ${HOOKSPATH}/ORDER ];then
    . ${HOOKS}/ORDER
fi

exit 0
