/* 
 * File:        :   ramdisk_device.h
 * Author       :   Tobias Sibera <Tobias.Sibera@HS-Osnabrueck.de>
 *                  Jens Overmoeller <Jens.Overmoeller@HS-Osnabrueck.de>
 * Copyright    :   GPLv3
 * Created on   :   08.01.2014, 20:43
 * Description  :   Repräsentiert die RAMDisk auf dem Device.
 */
#ifndef RAMDISK_DEVICE_H
#define	RAMDISK_DEVICE_H

// RAM-Settings
#define KERNEL_SECTOR_SIZE  512UL
#define LOGICAL_BLOCK_SIZE  524288UL    // 1024 X 512 KiB = 524288 Bits
#define NO_SECTORS          256UL       // (256 X 524288) / 1024 = 128 MB
// TOTAL SIZE = LOGICAL_BLOCK_SIZE * NO_SECTORS = 524288 * 256 = 134217728 Byte

static struct bd_device {
    unsigned long long size;
    spinlock_t lock;
    char *data;
    struct request_queue *queue;
    struct gendisk *disk;
} Device;

#endif
