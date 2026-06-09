#!/bin/bash

# flash_update.sh
# Usage: ./flash_update.sh <version> <filename> [QNX_IP]

VERSION=$1
FILENAME=$2
QNX_IP=${3:-10.0.0.1}

if [ -z "$VERSION" ] || [ -z "$FILENAME" ]; then
    echo "Usage: $0 <version> <filename> [QNX_IP]"
    exit 1
fi

echo "Starting flash update for version $VERSION ($FILENAME) from QNX gateway at $QNX_IP"

# Determine active partition
CMDLINE=$(cat /proc/cmdline)
if [[ $CMDLINE == *"root=/dev/mmcblk0p2"* ]]; then
    ACTIVE_PART="mmcblk0p2"
    INACTIVE_PART="mmcblk0p3"
elif [[ $CMDLINE == *"root=/dev/mmcblk0p3"* ]]; then
    ACTIVE_PART="mmcblk0p3"
    INACTIVE_PART="mmcblk0p2"
else
    echo "Could not determine active root partition. Defaulting to writing mmcblk0p3."
    ACTIVE_PART="mmcblk0p2"
    INACTIVE_PART="mmcblk0p3"
fi

echo "Active partition is $ACTIVE_PART"
echo "Flashing to inactive partition $INACTIVE_PART"

# Note: In a real system, you'd use ssh keys to avoid password prompts
echo "Copying $FILENAME from QNX and flashing to /dev/$INACTIVE_PART..."
ssh root@$QNX_IP "cat /tmp/$FILENAME" | dd of=/dev/$INACTIVE_PART bs=4M status=progress

if [ $? -eq 0 ]; then
    echo "Flash successful."
    
    # Update boot config (assuming standard RPi boot partition is mounted at /boot)
    # This is a simplified A/B boot switch for RPi
    if [ -f /boot/cmdline.txt ]; then
        sed -i "s/root=\/dev\/$ACTIVE_PART/root=\/dev\/$INACTIVE_PART/g" /boot/cmdline.txt
        echo "Updated cmdline.txt to boot from $INACTIVE_PART"
        
        # Reboot the system to apply
        echo "Rebooting in 5 seconds..."
        sleep 5
        # reboot
    else
        echo "Warning: /boot/cmdline.txt not found. Cannot switch boot partition automatically."
    fi
else
    echo "Flash failed!"
    exit 1
fi

exit 0
