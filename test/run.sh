#!/bin/bash

losetup /dev/loop0 disk.img -o $((2048 * 512))
file -s /dev/loop0
mount /dev/loop0 /mnt/os/
cp /home/olivier/os/kernel /mnt/os/boot/
umount /dev/loop0
losetup -d /dev/loop0

qemu -hda disk.img -m 512
