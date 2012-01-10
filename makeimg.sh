#!/bin/bash
 
# This script can be used to quickly test MultiBoot-compliant
# kernels.
 
# ---- begin config params ----
 
harddisk_image_size=$((16*128*4*512)) # however the hell big this is
harddisk_image="harddisk.img"
qemu_cmdline="qemu -monitor stdio"
kernel_args=""
kernel_binary="kernel.bin"
 
# ----  end config params  ----
 
 
function fail() { echo "$1"; sudo umount ./mnt; sudo /sbin/losetup -d /dev/loop1; sudo /sbin/losetup -d /dev/loop2; exit 1; }
function prereq() {
        local c x
        if [ "$1" = "f" ]; then c=stat;x=file; else c=which;x=program; fi
        if [ -z "$3" ]; then
                $c "$2" >/dev/null || fail "$x $2 not found"
        else
                $c "$2" >/dev/null || fail "$x $2 (from package $3) not found"
        fi
}
 
# check prerequisites
prereq x /sbin/mkfs.ext2
prereq x /sbin/grub-install
 
# create image
dd if=/dev/zero of="$harddisk_image" bs=4k count=$((harddisk_image_size/4096)) 2>/dev/null

# mount image
sudo /sbin/losetup /dev/loop1 "$harddisk_image" || fail "could not loop-mount harddisk.img"

# partition the image
sudo /sbin/sfdisk -S16 -H128 -C4 -uS /dev/loop1 <<END_PARTITION
256 7936
0 0
0 0
0 0
END_PARTITION

# mount the new partition
sudo /sbin/losetup -o 131072 /dev/loop2 harddisk.img || fail "Could not loop-setup the partition"

# format image
sudo /sbin/mkfs.ext2 /dev/loop2 || fail "could not format harddisk.img"

# mount the image
mkdir -p mnt || fail "could not create mount dir"
sudo mount /dev/loop2 ./mnt || fail "could not mount partition"

sudo mkdir -p ./mnt/boot/grub || fail "could not create boot directory"

sudo cp ./kernel ./mnt/boot/kernel || fail "could not copy kernel installation"

sudo /home/branan/prefix/grub2/sbin/grub-install --modules='ext2 part_msdos' --boot-directory=./mnt/boot /dev/loop1 || fail "could not install grub"

sudo bash -c 'cat > ./mnt/boot/grub/grub.cfg' <<END_MENUCFG
set default = '0'
set timeout = 2
menuentry '64BOSS' {
    set root='(hd0,1)'
    multiboot2 /boot/kernel
}
END_MENUCFG

sudo umount ./mnt
rmdir ./mnt
sudo losetup -d /dev/loop2
sudo losetup -d /dev/loop1
