__selected_layout = "cn"
__selected_layout_item = null
__selected_variant_item = null

__selected_timezone = "Asia/Shanghai"
__selected_username = null
__selected_hostname = null
__selected_password = null

__selected_locale = DCore.Installer.get_current_locale()

sync_installer_conf = ->
    DCore.Installer.record_accounts_info(__selected_username, __selected_hostname, __selected_password)

    DCore.Installer.record_timezone_info(__selected_timezone)

    DCore.Installer.record_locale_info(__selected_locale)

    if __selected_grub != "uefi"
        DCore.Installer.record_bootloader_info(__selected_grub, false)
    else
        esp = get_efi_boot_part()
        DCore.Installer.record_bootloader_info(esp, true)

    if __selected_layout.indexOf(",") != -1
        layout = __selected_layout.split(",")[0]
        variant = __selected_layout.split(",")[1]
    else
        layout = __selected_layout
        variant = null
    DCore.Installer.record_keyboard_layout_info(layout, variant)

    record_mount_points()

#write /etc/fstab, after extract iso
record_mount_points = ->
    for disk in disks
        for part in v_disk_info[disk]["partitions"]
            if v_part_info[part]["fs"] == "efi"
                v_part_info[part]["mp"] = "/boot/efi"
            if v_part_info[part]["fs"] == "swap"
                v_part_info[part]["mp"] = "swap"
            if v_part_info[part]["mp"]? and v_part_info[part]["mp"] != "unused"
                try
                    DCore.Installer.record_mountpoint_info(part, v_part_info[part]["mp"])

                catch error
                    echo error