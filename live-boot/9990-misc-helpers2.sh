#!/bin/sh

#set -e

check_dev ()
{
	sysdev="${1}"
	devname="${2}"
	skip_uuid_check="${3}"

	# support for fromiso=.../isofrom=....
	if [ -n "$FROMISO" ]
	then
		ISO_DEVICE=$(dirname $FROMISO)
		if ! [ -b $ISO_DEVICE ]
		then
			# to support unusual device names like /dev/cciss/c0d0p1
			# as well we have to identify the block device name, let's
			# do that for up to 15 levels
			i=15
			while [ -n "$ISO_DEVICE" ] && [ "$i" -gt 0 ]
			do
				ISO_DEVICE=$(dirname ${ISO_DEVICE})
				[ -b "$ISO_DEVICE" ] && break
				i=$(($i -1))
		        done
		fi

		if [ "$ISO_DEVICE" = "/" ]
		then
			echo "Warning: device for bootoption fromiso= ($FROMISO) not found.">>/boot.log
		else
			fs_type=$(get_fstype "${ISO_DEVICE}")
			if is_supported_fs ${fs_type}
			then
				mkdir /live/fromiso
				mount -t $fs_type "$ISO_DEVICE" /live/fromiso
				ISO_NAME="$(echo $FROMISO | sed "s|$ISO_DEVICE||")"
				loopdevname=$(setup_loop "/live/fromiso/${ISO_NAME}" "loop" "/sys/block/loop*" "" '')
				devname="${loopdevname}"
			else
				echo "Warning: unable to mount $ISO_DEVICE." >>/boot.log
			fi
		fi
	fi

	if [ -z "${devname}" ]
	then
		devname=$(sys2dev "${sysdev}")
	fi

	if [ -d "${devname}" ]
	then
		mount -o bind "${devname}" $mountpoint || continue

		if is_live_path $mountpoint
		then
			echo $mountpoint
			return 0
		else
			umount $mountpoint
		fi
	fi

	IFS=","
	for device in ${devname}
	do
		case "$device" in
			*mapper*)
				# Adding lvm support
				if [ -x /scripts/local-top/lvm2 ]
				then
					ROOT="$device" resume="" /scripts/local-top/lvm2 >>/boot.log
				fi
				;;

			/dev/md*)
				# Adding raid support
				if [ -x /scripts/local-top/mdadm ]
				then
					[ -r /conf/conf.d/md ] && cp /conf/conf.d/md /conf/conf.d/md.orig
					echo "MD_DEVS=$device " >> /conf/conf.d/md
					/scripts/local-top/mdadm >>/boot.log
					[ -r /conf/conf.d/md.orig ] && mv /conf/conf.d/md.orig /conf/conf.d/md
				fi
				;;
		esac
	done
	unset IFS

	[ -n "$device" ] && devname="$device"

	[ -e "$devname" ] || continue

	if [ -n "${LIVE_MEDIA_OFFSET}" ]
	then
		loopdevname=$(setup_loop "${devname}" "loop" "/sys/block/loop*" "${LIVE_MEDIA_OFFSET}" '')
		devname="${loopdevname}"
	fi

	fstype=$(get_fstype "${devname}")

	if is_supported_fs ${fstype}
	then
		devuid=$(blkid -o value -s UUID "$devname")
		[ -n "$devuid" ] && grep -qs "\<$devuid\>" /var/lib/live/boot/devices-already-tried-to-mount && continue
		mount -t ${fstype} -o ro,noatime "${devname}" ${mountpoint} || continue
		[ -n "$devuid" ] && echo "$devuid" >> /var/lib/live/boot/devices-already-tried-to-mount

		if [ -n "${FINDISO}" ]
		then
			if [ -f ${mountpoint}/${FINDISO} ]
			then
				umount ${mountpoint}
				mkdir -p /live/findiso
				
				for _p in $(cat /proc/cmdline);do
					case ${_p} in 
						auto-deepin-installer|auto-installer)
						RWFLAG="rw"
						;;
					esac
				done
				if [ "${RWFLAG}" = "rw" ];then
					if [ "${fstype}" = "ntfs" ];then
						modprobe fuse
						mount.ntfs-3g -o rw "${devname}" /live/findiso
					else
						mount -t ${fstype} -o rw,noatime "${devname}" /live/findiso
					fi
				else
					mount -t ${fstype} -o ro,noatime "${devname}" /live/findiso
				fi
				loopdevname=$(setup_loop "/live/findiso/${FINDISO}" "loop" "/sys/block/loop*" 0 "")
				devname="${loopdevname}"
				mount -t iso9660 -o ro,noatime "${devname}" ${mountpoint}
			else
				umount ${mountpoint}
			fi
		fi

		if is_live_path ${mountpoint} && \
			([ "${skip_uuid_check}" ] || matches_uuid ${mountpoint})
		then
			echo ${mountpoint}
			return 0
		else
			umount ${mountpoint} 2>/dev/null
		fi
	fi

	if [ -n "${LIVE_MEDIA_OFFSET}" ]
	then
		losetup -d "${loopdevname}"
	fi

	return 1
}
